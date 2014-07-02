#include "vtkSQLiteReader.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include <vtksys/ios/sstream>
#include <sstream>

#include <vtkStringList.h>
#include <vtkStdString.h>
#include <vtkStringArray.h>
#include <vtkDoubleArray.h>
#include <vtkCellArray.h>
#include <vtkPointData.h>

#include <vtkSQLQuery.h>
#include <vtkVariantArray.h>

#include <vtkSQLiteDatabase.h>
#include <vtkstd/map>


//----------------------------------------------------------------------------
struct Internal
{
	vtkstd::map < vtkStdString, vtkStdString > activeProps;

	vtkstd::map < int, vtkIntArray* > intArrayMap;
	vtkstd::map < int, vtkDoubleArray* > doubleArrayMap;
	vtkstd::map < int, vtkStringArray* > stringArrayMap;
};
//----------------------------------------------------------------------------


vtkCxxRevisionMacro(vtkSQLiteReader, "$Revision: 1.1 $");
vtkStandardNewMacro(vtkSQLiteReader);

//_________________________________________________________________________
vtkSQLiteReader::vtkSQLiteReader()
{
	this->FileName = NULL;
	this->Headers = NULL;
	this->guiLoaded = false;

	this->ActiveTable = NULL;
	this->ActiveTableType = NULL;
	this->Px = NULL;
	this->Py = NULL;
	this->Pz = NULL;

	this->SetNumberOfInputPorts(0);

	this->internals = new Internal();
}

//_________________________________________________________________________
vtkSQLiteReader::~vtkSQLiteReader()
{
	this->SetFileName(0);
	delete this->internals;
}

//_________________________________________________________________________
void vtkSQLiteReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent <<  "FileName: "
     << (this->FileName ? this->FileName : "(none)") << "\n";
}

//_________________________________________________________________________
int vtkSQLiteReader::CanReadFile( const char* fname )
{
	return 1;	
}

//_________________________________________________________________________
void vtkSQLiteReader::SetProperties(const char*name, const char* type, int status)
{
	if(status)
		this->internals->activeProps[name] = type;
	this->Modified();
}

//_________________________________________________________________________
int vtkSQLiteReader::RequestInformation(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector)
{
	// We only need to parse the headers before loading the GUI
	if(this->guiLoaded)
	{
		return 1;
	}
	this->guiLoaded = true;


	// Make sure we have a file to read.
	if(!this->FileName)  {
		vtkErrorMacro("No file name specified.  Cannot open.");
		return 0;
	}
	if(strlen(this->FileName)==0)  {
		vtkErrorMacro("File name is null.  Cannot open.");
		return 0;
	}

	vtkStdString fullName = "sqlite://" + vtkStdString(FileName);
	vtkSQLiteDatabase* db = vtkSQLiteDatabase::SafeDownCast( 
		vtkSQLDatabase::CreateFromURL( fullName.c_str() ) );
	bool status = db->Open("");
	if ( ! status )
	{
		vtkErrorMacro("Couldn't open database");
		return 0;
	}

	vtkStringArray* tables = db->GetTables();
	vtkStdString headers = "";
	for(unsigned int i=0; i<tables->GetNumberOfValues(); ++i)
	{
		vtkStdString tableName = tables->GetValue(i);
		headers += tableName + "::";
		
		vtkSQLQuery* sqlQuery = db->GetQueryInstance();
		vtkStdString query = "select * from " + tableName;
		sqlQuery->SetQuery(query.c_str());
		sqlQuery->Execute();

		for(int j=0; j<sqlQuery->GetNumberOfFields(); ++j)
		{
			vtkStdString fieldName = sqlQuery->GetFieldName(j);
			headers += fieldName;
			switch(sqlQuery->GetFieldType(j))
			{
			case VTK_INT:
				headers += "(int)";
				break;
			case VTK_FLOAT:
				headers +="(float)";
				break;
			case VTK_DOUBLE:
				headers += "(double)";
				break;
			case VTK_STRING:
				headers += "(string)";
				break;

			default:
				vtkErrorMacro("The type of " << fieldName.c_str() <<" "<< sqlQuery->GetFieldType(j) 
					<<"is unsupported");
			}
			if(j < sqlQuery->GetNumberOfFields()-1)
				headers += ",";
		}
		if(i < tables->GetNumberOfValues()-1)
			headers += "|";
	}

	this->Headers = new char[headers.size()];
	strcpy(this->Headers, headers.c_str());	

	db->Delete();
	return 1;
}


//_________________________________________________________________________
int vtkSQLiteReader::RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector)
{
	vtkInformation *outInfo = outputVector->GetInformationObject(0);
	vtkPolyData *output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

	// Make sure we have a file to read.
	if(!this->FileName)  {
		vtkErrorMacro("No file name specified.  Cannot open.");
		return 0;
	}
	if(strlen(this->FileName)==0)  {
		vtkErrorMacro("File name is null.  Cannot open.");
		return 0;
	}


	ifstream file;
	file.open(this->FileName, ios::in);
	if(!file)
	{
		vtkErrorMacro("File Error: cannot open file: "<< this->FileName);
		return 0;
	}

	vtkStdString fullName = "sqlite://" + vtkStdString(FileName);
	vtkSQLiteDatabase* db = vtkSQLiteDatabase::SafeDownCast( 
		vtkSQLDatabase::CreateFromURL( fullName.c_str() ) );
	bool status = db->Open("");
	if ( ! status )
	{
		vtkErrorMacro("Couldn't open database");
		return 0;
	}
	
	vtkSQLQuery* sqlQuery = db->GetQueryInstance();
	vtkStdString query = "select * from " + vtkStdString(this->ActiveTable);
	sqlQuery->SetQuery(query.c_str());
	sqlQuery->Execute();

	int xPos;
	int yPos;
	int zPos;

	for(int j=0; j<sqlQuery->GetNumberOfFields(); ++j)
	{
		vtkStdString fieldName = sqlQuery->GetFieldName(j);
		vtkstd::map<vtkStdString, vtkStdString>::iterator finder;
		finder = this->internals->activeProps.find(fieldName);
		if(finder != this->internals->activeProps.end())
		{
			if(finder->second == "int")
			{
				vtkIntArray* intArray = vtkIntArray::New();
				intArray->SetName(finder->first.c_str());
				this->internals->intArrayMap[j] = intArray;
			}
			else if(finder->second == "float" || finder->second == "double")
			{
				vtkDoubleArray* doubleArray = vtkDoubleArray::New();
				doubleArray->SetName(finder->first.c_str());
				this->internals->doubleArrayMap[j] = doubleArray;
			}
			else if(finder->second == "string")
			{
				vtkStringArray* stringArray = vtkStringArray::New();
				stringArray->SetName(finder->first.c_str());
				this->internals->stringArrayMap[j] = NULL;
			}
			else
			{
				vtkErrorMacro("Unsupported property type " << finder->first << ":" << finder->second);
			}
		}

		if(fieldName == vtkStdString(this->Px))
		{
			xPos = j;
		}
		// dont replace with "else if", since the same property can be used for x, y and/or z
		if(fieldName == vtkStdString(this->Py))
		{
			yPos = j;
		}
		if(fieldName == vtkStdString(this->Pz))
		{
			zPos = j;
		}
	}

	vtkPoints* outPoints = vtkPoints::New();
	vtkCellArray* outVerts = vtkCellArray::New();

	vtkVariantArray* vArray = vtkVariantArray::New();
	while(sqlQuery->NextRow(vArray))
	{
		for(vtkstd::map<int, vtkIntArray*>::iterator it = this->internals->intArrayMap.begin();
			it != this->internals->intArrayMap.end(); ++it)
		{
			it->second->InsertNextValue(vArray->GetValue(it->first).ToInt());
		}
		for(vtkstd::map<int, vtkDoubleArray*>::iterator it = this->internals->doubleArrayMap.begin();
			it != this->internals->doubleArrayMap.end(); ++it)
		{
			it->second->InsertNextValue(vArray->GetValue(it->first).ToDouble());
		}
		for(vtkstd::map<int, vtkStringArray*>::iterator it = this->internals->stringArrayMap.begin();
			it != this->internals->stringArrayMap.end(); ++it)
		{
			it->second->InsertNextValue(vArray->GetValue(it->first).ToString());
		}

		double pt[3];
		pt[0] = vArray->GetValue(xPos).ToDouble();
		pt[1] = vArray->GetValue(yPos).ToDouble();
		pt[2] = vArray->GetValue(zPos).ToDouble();

		vtkIdType pid = outPoints->InsertNextPoint(pt);
		outVerts->InsertNextCell(1);
		outVerts->InsertCellPoint(pid);
	}
	vArray->Delete();

	output->SetPoints(outPoints);
	output->SetVerts(outVerts);

	for(vtkstd::map<int, vtkIntArray*>::iterator it = this->internals->intArrayMap.begin();
		it != this->internals->intArrayMap.end(); ++it)
	{
		output->GetPointData()->AddArray(it->second);
		it->second->Delete();
	}
	for(vtkstd::map<int, vtkDoubleArray*>::iterator it = this->internals->doubleArrayMap.begin();
		it != this->internals->doubleArrayMap.end(); ++it)
	{
		output->GetPointData()->AddArray(it->second);
		it->second->Delete();
	}
	for(vtkstd::map<int, vtkStringArray*>::iterator it = this->internals->stringArrayMap.begin();
		it != this->internals->stringArrayMap.end(); ++it)
	{
		output->GetPointData()->AddArray(it->second);
		it->second->Delete();
	}

	db->Delete();
	this->internals->activeProps.clear();
	this->internals->intArrayMap.clear();
	this->internals->doubleArrayMap.clear();
	this->internals->stringArrayMap.clear();
	return 1;
}
