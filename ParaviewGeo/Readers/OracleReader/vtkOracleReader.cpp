#include "vtkOracleReader.h"
#include "StringUtilities.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include <vtksys/ios/sstream>
#include <vtkCellArray.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <iostream>
#include <fstream>

#include "ocilib.h"
#include <stdio.h>

using namespace std;

//_________________________________________________________________________
vtkCxxRevisionMacro(vtkOracleReader, "$Revision: 1.1 $");
vtkStandardNewMacro(vtkOracleReader);

//_________________________________________________________________________
vtkOracleReader::vtkOracleReader()
{
	this->FileName = NULL;
	this->SetNumberOfInputPorts(0);
}

//_________________________________________________________________________
vtkOracleReader::~vtkOracleReader()
{
	this->SetFileName(0);
}

//_________________________________________________________________________
void vtkOracleReader::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os,indent);
	os << indent <<  "FileName: "
		<< (this->FileName ? this->FileName : "(none)") << "\n";
}

//_________________________________________________________________________
int vtkOracleReader::CanReadFile( const char* fname )
{
	return 1;
}

//_________________________________________________________________________
int vtkOracleReader::RequestInformation(vtkInformation* request,
									  vtkInformationVector** inputVector,
									  vtkInformationVector* outputVector)
{
	return 1;
}

	//_________________________________________________________________________
int vtkOracleReader::RequestData(vtkInformation* request,
							   vtkInformationVector** inputVector,
							   vtkInformationVector* outputVector)
{
	ofstream logFile;
	logFile.open("OracleReader.log", 'w');

	vtkInformation *outInfo = outputVector->GetInformationObject(0);
	vtkPolyData *output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

	// open the <filename>.oracle parameters file
	ifstream inFile;
	inFile.open(this->FileName, ios::in);
	if(!inFile)
	{
		vtkErrorMacro("File Error: cannot open file: "<< this->FileName);
		return 0;
	}


	OCI_Connection* con;
	OCI_Statement* stmt;
	OCI_Resultset* res;

	string db;
	string user;
	string passwd;
	string tableName;
	string Px;
	string Py;
	string Pz;
	string propName;

	string line;
	vector<string> lineSplit;

	// username
	getline(inFile, line);
	StringUtilities::split(line, lineSplit, "=");
	user = lineSplit[1];

	// password
	getline(inFile, line);
	StringUtilities::split(line, lineSplit, "=");
	passwd = lineSplit[1];

	// oracle EZCONNECT string
	getline(inFile, line);
	StringUtilities::split(line, lineSplit, "=");
	db = lineSplit[1];

	// get the table name
	getline(inFile, line);
	StringUtilities::split(line, lineSplit, "=");
	tableName = lineSplit[1];

	// get the x values column name
	getline(inFile, line);
	StringUtilities::split(line, lineSplit, "=");
	Px = lineSplit[1];

	// get the y values column name
	getline(inFile, line);
	StringUtilities::split(line, lineSplit, "=");
	Py = lineSplit[1];

	// get the z values column name
	getline(inFile, line);
	StringUtilities::split(line, lineSplit, "=");
	Pz = lineSplit[1];

	// get the property name
	getline(inFile, line);
	StringUtilities::split(line, lineSplit, "=");
	propName = lineSplit[1];

	vtkDoubleArray* propArray = vtkDoubleArray::New();
	propArray->SetName(propName.c_str());
	if (!OCI_Initialize(NULL, NULL, OCI_ENV_DEFAULT | OCI_ENV_CONTEXT))
	{
		logFile << "Could not initialize environment" << endl;
		vtkErrorMacro("Could not initialize environment");
		return EXIT_FAILURE;
	}
	con = OCI_ConnectionCreate(db.c_str(), user.c_str(), passwd.c_str(), OCI_SESSION_DEFAULT);
	if (con == NULL)
	{
		logFile << "connection failed: connection string " << db << " is not working" << endl;
		vtkErrorMacro("connection failed: connection string " << db << " is not working");
		OCI_Error *err = OCI_GetLastError();
		if(err != NULL)
		{
			logFile << "errcode " << OCI_ErrorGetOCICode(err) << "  errmsg " << OCI_ErrorGetString(err) << endl;
			vtkErrorMacro("errcode " << OCI_ErrorGetOCICode(err) << "  errmsg " << OCI_ErrorGetString(err));
		}
		return 0;
	}
	else
	{
		logFile << "connection succeded" << endl;
		logFile << "Server version: major.minor.revision.connection = " << OCI_GetServerMajorVersion(con)
			 << "." << OCI_GetServerMinorVersion(con) << "." << OCI_GetServerRevisionVersion(con)
			 << "." << OCI_GetVersionConnection(con) << endl;

		vtkWarningMacro("Server version: major.minor.revision.connection = " << OCI_GetServerMajorVersion(con)
			 << "." << OCI_GetServerMinorVersion(con) << "." << OCI_GetServerRevisionVersion(con)
			 << "." << OCI_GetVersionConnection(con));
	}

	stmt = OCI_StatementCreate(con);
	

	// select desired values
	string sql = "select " + Px + ", " + Py + ", " + Pz + ", " + propName + " from " + tableName;
	if (!OCI_ExecuteStmt(stmt, sql.c_str()))
	{
		logFile << "Could not execute SQL statement: " << sql << endl;
		vtkErrorMacro("Could not execute SQL statement: " << sql);
		return 0;
	}
	
    OCI_Statement *st;
    OCI_Resultset *rs;
	st = OCI_StatementCreate(con);

	string statement = "select * from " + tableName;
	OCI_ExecuteStmt(st, statement.c_str());

    rs = OCI_GetResultset(st);
    int nb = OCI_GetColumnCount(rs);
    
	logFile << endl << "column: name, type, size, precision, scale" << endl;
	vtkWarningMacro("column: name, type, size, precision, scale");
    for(int i = 1; i <= nb; i++)
    {
		OCI_Column* col = OCI_GetColumn(rs, i);
     
		vtkWarningMacro("" << OCI_GetColumnName(col) << ", " << OCI_GetColumnSQLType(col)
			<< ", " << OCI_GetColumnSize(col) << ", " << OCI_GetColumnSize(col) 
			<< ", " << OCI_GetColumnPrecision(col) << ", " << OCI_GetColumnScale(col));
		
		logFile << OCI_GetColumnName(col) << ", " << OCI_GetColumnSQLType(col)
			<< ", " << OCI_GetColumnSize(col) << ", " << OCI_GetColumnSize(col) 
			<< ", " << OCI_GetColumnPrecision(col) << ", " << OCI_GetColumnScale(col) << endl;
    }

	// Get the result set and get the desired data
	vtkPoints* outPoints = vtkPoints::New();
	vtkCellArray* outVerts = vtkCellArray::New();
	res = OCI_GetResultset(stmt);
	while(OCI_FetchNext(res))
	{
		double pt[3];
		// add 1 since they start at 1, not 0
		pt[0] = OCI_GetDouble(res, 1);
		pt[1] = OCI_GetDouble(res, 2);
		pt[2] = OCI_GetDouble(res, 3);

		double propValue = OCI_GetDouble(res, 4);

		vtkIdType pid = outPoints->InsertNextPoint(pt);
		outVerts->InsertNextCell(1);
		outVerts->InsertCellPoint(pid);

		propArray->InsertNextValue(propValue);
	}

	output->SetPoints(outPoints);
	output->SetVerts(outVerts);
	output->GetPointData()->AddArray(propArray);

	OCI_Cleanup();
	
	logFile.close();
	return 1;
}