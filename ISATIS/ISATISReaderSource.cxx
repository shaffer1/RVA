/*=========================================================================

Program:   RVA
Module:    ISATISReader

Copyright (c) University of Illinois at Urbana-Champaign (UIUC)
Original Authors: L Angrave, J Duggirala, D McWherter, U Yadav

All rights reserved.
See Copyright.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN // Don't use bloated ver of windows.h
#include <windows.h> // Needed for windows Sleep()
#include <process.h> // Contains _beginthread(ex) and _endthread(ex)
#undef GetMessage // This breaks GTXError::GetMessage() (defined by Windows.h)
#endif // _WIN32

#include <cassert>

#include "ISATISReaderSource.h"
#include "ISATISReaderDelegate.h"
#include "ISATISReaderDefault.h"
#include "ISATISReaderGrid.h"
#include "ISATISReaderLine.h"
#include "ISATISReaderPolygon.h"
#include "ISATISFaultProcessor.h"

#include "vtkDataArray.h"
#include "vtkDataObject.h"
#include "vtkDoubleArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkStreamingDemandDrivenPipeline.h"

#include <GTXDoubleData.hpp>
#include <GTXError.hpp>
#include <GTXFaultInfo.hpp>
#include <GTXFileInfo.hpp>
#include <GTXVariableInfo.hpp>

#define CONNECT_SUCCEEDED (7) /* A non zero, non-259 value */

vtkStandardNewMacro(ISATISReaderSource);
//----------------------------------------------------------------------------

static void copyToVtkStringArray(const GTXStringArray&in, vtkStringArray*out)
{
  assert(out);
  out->Reset();

  const int num = in.GetCount();
  const char ** raw = in.GetValues();
  assert(raw != NULL ||  num == 0);

  for(int i =0; i<num; i++)
    out->InsertNextValue( raw[i] );

}

//----------------------------------------------------------------------------
static ISATISReaderDefault* DefaultDelegate = ISATISReaderDefault::New();

static ISATISReaderDelegate* DELEGATES[] = {
  (ISATISReaderDelegate*) ISATISReaderGrid::New(),
  (ISATISReaderDelegate*) ISATISReaderLine::New(),
  (ISATISReaderDelegate*) ISATISReaderPolygon::New(),
};

#define ARRAYLENGTHMACRO(array) (sizeof(array) / sizeof(array[0]) )

ISATISReaderDelegate* ISATISReaderSource::ChooseDelegate()
{
  GTXFileInfo fileInfo = this->Client->GetFileInfo();

  int num_delegates = ARRAYLENGTHMACRO(DELEGATES) ;

  for(int i=0; i<num_delegates ; i++) {
    if(DELEGATES[i]->CanRead(this,this->Client,&fileInfo))
      return DELEGATES[i];
  }
  return DefaultDelegate;
}

//----------------------------------------------------------------------------
ISATISReaderSource::ISATISReaderSource()
{
  this->SetDebug(1);
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(2);

  this->GTXServerPort=-1;
  this->GTXServerHost = "";// e.g. "127.0.0.1";
  this->GTXServerPath ="";
  this->GTXStudy  = ""; //eg "RVA_Project2";
  this->GTXDirectory = ""; //eg "Grids";
  this->GTXFileName = ""; // eg RVA2_Strat";
  this->GTXLengthUnit="";

  this->Client=new GTXClient; // We need this first thing, so simply init

  this->StudyNames = vtkStringArray::New();
  this->DirectoryNames = vtkStringArray::New();
  this->FileNames = vtkStringArray::New();

}

//----------------------------------------------------------------------------
ISATISReaderSource::~ISATISReaderSource()
{
  Disconnect();
  delete Client;
  Client=0;
  this->StudyNames->Delete();
  this->DirectoryNames->Delete();
  this->FileNames->Delete();
}

//----------------------------------------------------------------------------
void ISATISReaderSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void ISATISReaderSource::SetGTXLengthUnit(vtkStdString unit)
{
  // unit var must either be empty string or a valid unit. e.g. "cm (blah)"
  int pos =0;
  if(!unit.empty()) {
    pos = unit.find( " (" , 0 );
    assert(pos>0);
  }

  vtkStdString shortUnit = (pos>0) ? unit.substr(0,pos) : this->GTXLengthUnit; // extract the units before " (blah")

  if(this->GTXLengthUnit != shortUnit) {
    this->GTXLengthUnit = shortUnit;
    this->Modified();
  }
}
#ifdef _WIN32
unsigned int __stdcall ISATISReaderSource::TryConnection(void* data)
{
  assert(data); // Should never get a NULL ptr
  ISATISReaderSource * src = static_cast<ISATISReaderSource*>(data);

  assert(src && src->GTXServerHost && src->GTXServerPath && src->GTXServerPort && src->Client);

  try {
    src->Client->Connect(src->GTXServerHost, src->GTXServerPort, src->GTXServerPath);
    if(! src->Client->IsConnected() )
      return 0;
  } catch(...) {
    return 0;
  }

  return CONNECT_SUCCEEDED; // must not be 259... Windows uses this value to mean thread still running
}
#endif



//----------------------------------------------------------------------------
void ISATISReaderSource::Connect(bool forceNewConnection)
{
  if(forceNewConnection && Client->IsConnected())
    Disconnect();

  this->StudyNames->Reset();
  try {
    // we'll exercise our supposed connection by asking for a list of studies
    this->ServerStatus = NoConnection;
    this->StatusMessage =  "Could not connect";

    bool succeeded = false;

    // GTXClient users Winsock 1.1 which does not support setting short timeout
    //  on connect
#ifdef _WIN32
    unsigned int threadId = 0;

    // CreateThread() is the raw Win32 API call - use _beginthreadex instead (safer/does additional bookkeeping)
    void * hThread = (void*)_beginthreadex(0, 0, &ISATISReaderSource::TryConnection, this, 0, &threadId);
    assert(hThread);
    assert(threadId);
    if(hThread) { // Paranoid test (should always be true)
      bool threadFinished =  WaitForSingleObject(hThread, 5000) == 0L; // 0L=WAIT_OBJECT_0
      if(threadFinished) {
        DWORD exitCode=259; // 259 = Windows code for Still ACTIVE
        GetExitCodeThread(hThread,&exitCode);
        succeeded = exitCode == CONNECT_SUCCEEDED;
      }

      // For the paranoid refer to http://msdn.microsoft.com/en-us/library/ms686717%28v=vs.85%29.aspx
      // But heap smashing as has not been observed in this application
      TerminateThread(hThread, 0 /* exit code */);

      CloseHandle(hThread); // This only closes the handle
      hThread = 0;
    }
#else
    this->Client->Connect(this->GTXServerHost, this->GTXServerPort, this->GTXServerPath);
    succeeded = Client->IsConnected();
#endif // _WIN32

    // Validate our connection
    // We also use the study names for the client drop down

    GTXStringArray studies = Client->GetStudyList();
    copyToVtkStringArray(studies, this->StudyNames);

  } catch(GTXError& e) {
    vtkDebugMacro(<<e.GetMessage());
    return;
  }

  if(! Client->IsConnected() || this->StudyNames->GetSize()==0)
    return ;

  this->ServerStatus = Connected;
  this->StatusMessage = "Connected";

  if(! SetupClient())
    return ;

  this->ServerStatus = ItemAvailable;
}



int ISATISReaderSource::SetupClient()
{
  try {

    this->DirectoryNames->Reset();
    this->FileNames->Reset();

    if(this->GTXStudy.empty()) return 0;
    Client->SetStudy(this->GTXStudy);

    GTXStringArray dirs = Client->GetDirectoryList();
    copyToVtkStringArray(dirs, this->DirectoryNames);


    if(this->GTXDirectory.empty()) return 0;
    Client->SetDirectory(this->GTXDirectory);

    GTXStringArray allFiles = Client->GetFileList();

    // Only add readable files e g 2D Grids should be ignored
    const int num = allFiles.GetCount();
    const char ** raw = allFiles.GetValues();
    assert(raw);
    for(int i =0; i<num; i++) {
      Client->SetFile(raw[i]);
      if(ChooseDelegate() != DefaultDelegate)
        this->FileNames->InsertNextValue( raw[i] );
    }

    if(this->GTXFileName.empty()) return 0;
    Client->SetFile(this->GTXFileName);
    if(this->GTXLengthUnit.empty()) {
      Client->SetUnitMode(1); // vars use their own length scales

    } else {
      Client->SetUnitMode(0); // conform to the  length units
      Client->SetUnit(this->GTXLengthUnit);
    }
  } catch(GTXError& e) {
    vtkDebugMacro(<<e.GetMessage());
    return 0;
  }
  return 1; // Managed to set the file
}

//----------------------------------------------------------------------------
void ISATISReaderSource::Disconnect()
{
  try {
    this->Client->Disconnect();
  } catch(GTXError& e) {
    vtkDebugMacro(<<e.GetMessage());  // For debug purposes
  }
}


//----------------------------------------------------------------------------

vtkStringArray* ISATISReaderSource::GetStudyNames()
{
  //xxx GTXConnectionValidityEnum status = Connect(true); // true = new conection. Do we really need a new connection?
  return this->StudyNames;
}

vtkStringArray* ISATISReaderSource::GetDirectoryNames()
{
  SetupClient();
  return this->DirectoryNames;
}

vtkStringArray* ISATISReaderSource::GetFileNames()
{
  SetupClient();
  return this->FileNames;
}

//----------------------------------------------------------------------------

int ISATISReaderSource::FillOutputPortInformation(int port, vtkInformation* info)
{
  // NB We're only called once
  if (port == 0)
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkDataSet");
  else
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");

  return 1;
}
//----------------------------------------------------------------------------

int ISATISReaderSource::ProcessRequest(vtkInformation* request,
                                       vtkInformationVector** inputVector,
                                       vtkInformationVector* outputVector)
{
  if(!request || !outputVector || !this->Client)
    return 0; // Paranoia e.g. this object deleted but still in pipeline

  if(request->Has(vtkDemandDrivenPipeline::REQUEST_DATA_OBJECT()))
    return RequestDataObject(request,inputVector,outputVector);
  if(request->Has(vtkDemandDrivenPipeline::REQUEST_INFORMATION()))
    return RequestInformation(request,inputVector,outputVector);
  if(request->Has(vtkDemandDrivenPipeline::REQUEST_DATA()))
    return RequestData(request,inputVector,outputVector);

  return this->Superclass::ProcessRequest(request, inputVector, outputVector);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

int ISATISReaderSource::RequestDataObject(vtkInformation* request,
    vtkInformationVector** inputVector,
    vtkInformationVector* outputVector)

{
  // Phase 1. Try to connect to GTX. This will fail if the server is down or the
  // study/directory/filename are invalid
  this->StatusMessage.clear();
  this->Delegate = DefaultDelegate;

  // Phase 2. If gtx client is looking at a valid file then try to choose a delegate
  // that can read it.
  if(this->ServerStatus == ItemAvailable) {
    try {
      this->Delegate = ChooseDelegate();
    }	catch(GTXError& e) {
      this->StatusMessage = e.GetMessage();
      vtkErrorMacro(<<e.GetMessage());
    }
  }

  // Phase 3. Set the desired output object
  int result = 0; // 0 = failed
  try {
    result=this->Delegate->RequestDataObject(request,outputVector,this,this->Client);
  } catch (GTXError& e) {
    this->StatusMessage = e.GetMessage();
    vtkErrorMacro(<<e.GetMessage());
  }

  // Phase 4. The Delegate failed, time to let the default delegate clean up the mess.
  if(result==0 && Delegate != DefaultDelegate) {
    vtkErrorMacro(<<"Delegate failed so using Default Delegate");
    this->Delegate = DefaultDelegate;
    result=this->Delegate->RequestDataObject(request,outputVector,this,this->Client);
  }
  return result;
}
//----------------------------------------------------------------------------
int ISATISReaderSource::RequestInformation(vtkInformation* request,
    vtkInformationVector** inputVector,
    vtkInformationVector* outputVector)

{
  int result = 0; // 0 = failed
  try {
    result= this->Delegate->RequestInformation(request,outputVector,this,this->Client);
  } catch (GTXError& e) {
    vtkErrorMacro(<<e.GetMessage());
    if(Delegate != DefaultDelegate) {
      Delegate = DefaultDelegate;
      result=this->Delegate->RequestInformation(request,outputVector,this,this->Client);
    }
  }
  return result;
}

//----------------------------------------------------------------------------
int ISATISReaderSource::RequestData(vtkInformation* request,
                                    vtkInformationVector** inputVector,
                                    vtkInformationVector* outputVector)

{
  int result = 0;
  try {
    result = this->Delegate->RequestData(request,outputVector,this,this->Client);
  } catch(GTXError& e) {
    vtkErrorMacro(<<e.GetMessage());
    if(Delegate != DefaultDelegate) {
      Delegate = DefaultDelegate;
      result=this->Delegate->RequestData(request,outputVector,this,this->Client);
    }
  }

  // We handle faults here because they can be associated with any kind of GTX data
  if (result == 1 && Client->GetFileInfo().GetFaultedFlag()){
    double extendZRange = 0.05; // make Faults extend past the min max Z range by 5%
    ISATISFaultProcessor faultProcessor(Delegate->minZ, Delegate->maxZ, extendZRange);

    // Maybe this priority could be from the GUI.
    int faultPriority = Client->GetFileInfo().GetFaultInfo().GetAuthorizedPriority();
    GTXFaultSystem faultSystem = Client->ReadFaults(faultPriority); 
    return faultProcessor.process(outputVector, &faultSystem);
  } else{
    return result;
  }
}
