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

// .NAME ISATISReaderSource - Read ISATIS Data.
// .SECTION Description
// ISATISReaderSource is the factory and essential component
// to reading ISATIS data into ParaView. It relies on having
// a valid connection to a GTXserver (currently suppported is
// version 2011.2) and appropriately chooses the correct delegate
// to read the data based on the data format.
// .SECTION See Also
// ISATISReaderDelegate
// ISATISReaderDefault
// ISATISReaderGrid
// ISATISReaderLine
// ISATISReaderPolygon

#ifndef __ISATISReaderSource_h
#define __ISATISReaderSource_h

#include <GTXClient.hpp>
#include "vtkAlgorithm.h"
#include "vtkStringArray.h" // vtkStdString.h implicitly included

class ISATISReaderDelegate;

enum GTXConnectionValidityEnum { NoConnection, Connected, ItemAvailable=100 };

class ISATISReaderSource : public vtkAlgorithm {
public:
  // Description:
  // Create an object with Debug turned off, modified time initialized to zero, and reference counting on.
  static ISATISReaderSource *New();

  // Description:
  // If class is of (or subclass of) the specified type
  // return 1. Otherwise, return 0.
  vtkTypeMacro(ISATISReaderSource,vtkAlgorithm);

  // Description:
  // Method used by print. Typically not called by user.
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Public set method to alter object information.
  vtkSetMacro(GTXServerPort,int);
  vtkSetMacro(GTXServerHost,vtkStdString);
  vtkSetMacro(GTXServerPath,vtkStdString);
  vtkSetMacro(GTXStudy,vtkStdString);
  vtkSetMacro(GTXDirectory,vtkStdString);
  vtkSetMacro(GTXFileName,vtkStdString);

  // Description:
  // Determines the unit of length (i.e. m, ft, km, etc.) to appropriately
  // scale the data for display in ParaView.
  virtual void SetGTXLengthUnit(vtkStdString unit);

  //vtkGetMacro(StudyNames,vtkStringArray*);

  // Description:
  // Returns an array of study, directory, or file names currently
  // valid on the server (respectively).
  vtkStringArray* GetStudyNames();
  vtkStringArray* GetDirectoryNames();
  vtkStringArray* GetFileNames();

  // Description:
  // Determines which method should be called at a
  // given stage in the pipeline to appropriately retrieve,
  // initialize, and create data in ParaView. Upon selecting
  // a valid method, its result is returned. If there is
  // no valid option, vtkAlgorithm::ProcessRequest will be
  // called and its result is returned.
  // \li 0 = Failure
  // \li 1 = Success
  virtual int ProcessRequest(vtkInformation* request,
                             vtkInformationVector** inputVector,
                             vtkInformationVector* outputVector);

  // Description:
  // Prepares ParaView's output port for the appropriate
  // data type to be displayed. Always returns 1.
  virtual int FillOutputPortInformation(int port, vtkInformation* info);

protected:

  ISATISReaderSource();
  ~ISATISReaderSource();

  // Description:
  // Try to connect to a valid server and choose the
  // appropriate delegate for reading requested data.
  // Returns an integer status code:
  // \li 0 = Failure
  // \li 1 = Success
  virtual int RequestDataObject(vtkInformation* request,
                                vtkInformationVector** inputVector,
                                vtkInformationVector* outputVector);

  // Description:
  // Helper function which calls the RequestInformation(...)
  // (or RequestData(...) respectively) method on the appropriate
  // delegate returning the delegate's response.
  // \li 0 = Failure
  // \li 1 = Success</list>\n
  // See Also:
  // ISATISReaderGrid
  // ISATISReaderLine
  // ISATISReaderPolygon
  virtual int RequestInformation(vtkInformation* request,
                                 vtkInformationVector** inputVector,
                                 vtkInformationVector* outputVector);
  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);

private:
  friend class ISATISReaderDelegate;
  friend class ISATISGUIPanel;

  // Description:
  // Testing function (unnecessary for general use)
  friend int TestISATISReader(int,char**);
  friend void SimulatePipeline(ISATISReaderSource*,vtkInformationVector*);

  ISATISReaderSource(const ISATISReaderSource&);  // Not implemented.
  void operator=(const ISATISReaderSource&);  // Not implemented.

  // Description:
  // Attempts to initialize a connection with the specified
  // GTXserver. Sets the status code based on the status of
  // the connection.
  // \li ISATISReaderSource::NoConnection  = No connection could be made
  // \li ISATISReaderSource::Connected     = Successfully initialized connection
  // \li ISATISReaderSource::ItemAvailable = Successful connection and items are available
  void Connect(bool forceNewConnection);

  // Description:
  // Terminates (if applicable) the existing connection
  // to the GTXserver.
  void Disconnect();

  // Description:
  // Attempts to initialize a valid connection with a specified
  // GTXserver. It returns an integer using the codes below:
  // \li 0 = Failure
  // \li 1 = Success (Item found)
  int SetupClient(); // 0 = Failure, 1 = Success (Item found)

  // Description:
  // Returns (if available) an appropriate delegate to handle
  // the proposed data type from GTXserver. If no there is no
  // applicable candidate, the default delegate is returned.
  ISATISReaderDelegate* ChooseDelegate();

#ifdef _WIN32
  // Description:
  // Win32 thread call to sample connection and terminate
  // if no valid response within predefined time limit
  //BTX
  static unsigned int __stdcall TryConnection(void*);
  //ETX
#endif // _WIN32

  int GTXServerPort;
  vtkStdString GTXServerHost;
  vtkStdString GTXServerPath;
  vtkStdString GTXStudy;
  vtkStdString GTXDirectory;
  vtkStdString GTXFileName;
  vtkStdString StatusMessage;
  vtkStdString GTXLengthUnit;

  vtkStringArray *StudyNames;
  vtkStringArray *DirectoryNames;
  vtkStringArray *FileNames;

  GTXClient *Client;
  ISATISReaderDelegate* Delegate;

  GTXConnectionValidityEnum ServerStatus;
};


#endif


