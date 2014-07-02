#include "vtkFLAC3DReader.h"

#include "vtkUnstructuredGrid.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include <vtksys/ios/sstream>
#include <sstream>

#include <vtkStringList.h>
#include <vtkStdString.h>
#include <vtkStringArray.h>
#include <vtkIntArray.h>
#include <vtkDoubleArray.h>
#include <vtkCellArray.h>
#include <vtkPointData.h>
#include <vtkCellData.h>

#include <map>

#include "StringUtilities.h"

using std::map;

namespace
{
	map < vtkIdType, vtkIdType > pidMap;
	map < vtkIdType, vtkIdType > cidMap;
}


vtkCxxRevisionMacro(vtkFLAC3DReader, "$Revision: 1.1 $");
vtkStandardNewMacro(vtkFLAC3DReader);

//_________________________________________________________________________
vtkFLAC3DReader::vtkFLAC3DReader()
{
	this->FileName = NULL;
	this->SetNumberOfInputPorts(0);

	this->DisplacementVectorsFile = NULL;
	this->ZonesScalarsFile = NULL;
	this->ZonesTensorsFile = NULL;
}

//_________________________________________________________________________
vtkFLAC3DReader::~vtkFLAC3DReader()
{
	this->SetFileName(NULL);
}

//_________________________________________________________________________
void vtkFLAC3DReader::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os,indent);
	os << indent <<  "FileName: "
		<< (this->FileName ? this->FileName : "(none)") << "\n";
}

//_________________________________________________________________________
int vtkFLAC3DReader::CanReadFile( const char* fname )
{
	return 1;	
}

//_________________________________________________________________________
int vtkFLAC3DReader::RequestInformation(vtkInformation* request,
									  vtkInformationVector** inputVector,
									  vtkInformationVector* outputVector)
{
	return 1;
}


//_________________________________________________________________________
int vtkFLAC3DReader::RequestData(vtkInformation* request,
							   vtkInformationVector** inputVector,
							   vtkInformationVector* outputVector)
{
	vtkInformation *outInfo = outputVector->GetInformationObject(0);
	vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

	// Make sure we have a file to read.
	if(!this->FileName)  {
		vtkErrorMacro("No file name specified.  Cannot open.");
		return 0;
	}
	if(strlen(this->FileName)==0)  {
		vtkErrorMacro("File name is null.  Cannot open.");
		return 0;
	}


	ifstream inFile;
	inFile.open(this->FileName, ios::in);
	if(!inFile)
	{
		vtkErrorMacro("File Error: cannot open file: "<< this->FileName);
		return 0;
	}

	vtkPoints* outPoints = vtkPoints::New();

	vtkIdType ids[8];
	string line;
	vector<string> lineSplit;

	pidMap.clear();
	cidMap.clear();

	// skip the headers
	while(!inFile.eof())
	{
		getline(inFile, line);
		if(line.size() == 0) continue;
		if(line[0] == '*') continue;
		if(line[0] == 'G')
			break;
	}

	// read the GRIDPOINTS section
	while(line.substr(0,2) == "G ")
	{
		StringUtilities::split(line, lineSplit, " ");

		vtkIdType id;
		double pt[3];
		id = atoi(lineSplit[1].c_str());
		pt[0] = atof(lineSplit[2].c_str());
		pt[1] = atof(lineSplit[3].c_str());
		pt[2] = atof(lineSplit[4].c_str());

		vtkIdType newId = outPoints->InsertNextPoint(pt);
		pidMap[id] = newId;

		if(inFile.eof()) break;

		do
		{
			getline(inFile, line);
		}while((line.size() == 0 || line[0] == '*') &! inFile.eof());

		if(line.size() == 0 || line[0] == '*')
			break;
	}

	if(line.size() == 0 || line[0] == '*' || inFile.eof())
	{
		output->SetPoints(outPoints);
		outPoints->Delete();
		return 1;
	}

	vtkUnstructuredGrid* temp= vtkUnstructuredGrid::New();

	// read the ZONES section
	while(line.substr(0,2) == "Z ")
	{
		StringUtilities::split(line, lineSplit, " ");

		if(lineSplit[1] == "B8")
		{
			vtkIdType id = atoi(lineSplit[2].c_str());

			// see FLAC3D documentation for hexahedron topology
			ids[0] = atoi(lineSplit[3].c_str());
			ids[1] = atoi(lineSplit[4].c_str());
			ids[2] = atoi(lineSplit[7].c_str());
			ids[3] = atoi(lineSplit[5].c_str());
			ids[4] = atoi(lineSplit[6].c_str());
			ids[5] = atoi(lineSplit[9].c_str());
			ids[6] = atoi(lineSplit[10].c_str());
			ids[7] = atoi(lineSplit[8].c_str());

			map <vtkIdType, vtkIdType>::iterator it;
			it = pidMap.find(ids[0]);
			if(it == pidMap.end())
				throw("point id missing, topology can't be created");
			ids[0] = it->second;

			it = pidMap.find(ids[1]);
			if(it == pidMap.end())
				throw("point id missing, topology can't be created");
			ids[1] = it->second;

			it = pidMap.find(ids[2]);
			if(it == pidMap.end())
				throw("point id missing, topology can't be created");
			ids[2] = it->second;

			it = pidMap.find(ids[3]);
			if(it == pidMap.end())
				throw("point id missing, topology can't be created");
			ids[3] = it->second;

			it = pidMap.find(ids[4]);
			if(it == pidMap.end())
				throw("point id missing, topology can't be created");
			ids[4] = it->second;

			it = pidMap.find(ids[5]);
			if(it == pidMap.end())
				throw("point id missing, topology can't be created");
			ids[5] = it->second;

			it = pidMap.find(ids[6]);
			if(it == pidMap.end())
				throw("point id missing, topology can't be created");
			ids[6] = it->second;

			it = pidMap.find(ids[7]);
			if(it == pidMap.end())
				throw("point id missing, topology can't be created");
			ids[7] = it->second;

			vtkIdType newId = temp->InsertNextCell(VTK_HEXAHEDRON,8,ids);
			cidMap[id] = newId;
		}

		if(inFile.eof()) break;

		do
		{
			getline(inFile, line);
		}while((line.size() == 0 || line[0] == '*') &! inFile.eof());

		if(line.size() == 0 || line[0] == '*')
			break;
	}

	temp->SetPoints(outPoints);
	output->ShallowCopy(temp);
	temp->Delete();
	outPoints->Delete();

	if(line.size() == 0 || line[0] == '*' || inFile.eof())
		return 1;

	vtkStringArray* groupNameArray = vtkStringArray::New();
	groupNameArray->SetName("Group Name");
	groupNameArray->SetNumberOfValues(output->GetNumberOfCells());

	vtkIntArray* groupIdArray = vtkIntArray::New();
	groupIdArray->SetName("Group Id");
	groupIdArray->SetNumberOfValues(output->GetNumberOfCells());

	// in case some zones doesn't belong to a group
	// if it's sure that all zones belong to a group, this
	// loop should be removed to save time
	//for(vtkIdType i=0; i<output->GetNumberOfCells(); ++i)
	//{
	//	groupNameArray->SetValue(i, "");
	//	groupIdArray->SetValue(i,-1);
	//}

	int groupId = 0;
	while(line.substr(0,7) == "ZGROUP ")
	{
		StringUtilities::split(line, lineSplit, " ");
		string groupName = lineSplit[1].substr(1,lineSplit[1].size()-2);

		while(true)
		{
			do
			{
				getline(inFile, line);
			}while((line.size() == 0 || line[0] == '*') &! inFile.eof());

			if(line.size() == 0 || line[0] == '*' || inFile.eof() || line.substr(0,6) == "ZGROUP")
				break;
			
			StringUtilities::split(line, lineSplit, " ");
			for(vector<string>::iterator it = lineSplit.begin(); it != lineSplit.end(); ++it)
			{
				int zone = atoi(it->c_str());

				map <vtkIdType, vtkIdType>::iterator mit;
				mit = cidMap.find(zone);
				if(mit == cidMap.end())
					throw("zone id missing does not exist");
				zone = mit->second;

				groupNameArray->SetValue(zone, groupName);
				groupIdArray->SetValue(zone, groupId);
			}
		}
		++groupId;
	}

	output->GetCellData()->AddArray(groupNameArray);
	output->GetCellData()->AddArray(groupIdArray);

	groupNameArray->Delete();
	groupIdArray->Delete();

	inFile.close();

	this->importDisplacements(output);
	this->importScalars(output);
	this->importTensors(output);

	return 1;
}



//_________________________________________________________________________
void vtkFLAC3DReader::importDisplacements(vtkUnstructuredGrid *output)
{
	// Make sure we have a file to read.
	if(!this->DisplacementVectorsFile)  {
		return;
	}
	if(strlen(this->DisplacementVectorsFile)==0)  {
		return;
	}


	ifstream inFile;
	inFile.open(this->DisplacementVectorsFile, ios::in);
	if(!inFile)
	{
		vtkErrorMacro("File Error: cannot open file: "<< this->DisplacementVectorsFile);
		return;
	}

	vtkDoubleArray* dispArray = vtkDoubleArray::New();
	dispArray->SetName("Displacement Vectors");
	dispArray->SetNumberOfComponents(3);
	
	dispArray->SetNumberOfTuples(output->GetNumberOfPoints());
	for(vtkIdType i=0; i<output->GetNumberOfPoints(); ++i)
	{
		dispArray->SetTuple3(i,0,0,0);
	}

	string line;
	vector<string> lineSplit;

	// skip the headers
	while(!inFile.eof())
	{
		getline(inFile, line);
		if(line.size() > 0)
			break;
	}

	while(!inFile.eof())
	{
		getline(inFile, line);
		if(line.size() < 4)
			continue;

		StringUtilities::split(line, lineSplit, ";");
		vtkIdType id = atoi(lineSplit[0].c_str());
		double dispX = atof(lineSplit[1].c_str());
		double dispY = atof(lineSplit[2].c_str());
		double dispZ = atof(lineSplit[3].c_str());

		map <vtkIdType, vtkIdType>::iterator it;
		it = pidMap.find(id);
		if(it == pidMap.end())
			throw("point id not found");
		id = it->second;

		dispArray->SetTuple3(id, dispX, dispY, dispZ);
	}

	inFile.close();

	output->GetPointData()->AddArray(dispArray);
	dispArray->Delete();
}

//_________________________________________________________________________
void vtkFLAC3DReader::importScalars(vtkUnstructuredGrid *output)
{
	// Make sure we have a file to read.
	if(!this->ZonesScalarsFile)  {
		return;
	}
	if(strlen(this->ZonesScalarsFile)==0)  {
		return;
	}


	ifstream inFile;
	inFile.open(this->ZonesScalarsFile, ios::in);
	if(!inFile)
	{
		vtkErrorMacro("File Error: cannot open file: "<< this->ZonesScalarsFile);
		return;
	}

	vtkIntArray* stateArray = vtkIntArray::New();
	stateArray->SetName("state");

	vtkDoubleArray* cohesionArray = vtkDoubleArray::New();
	cohesionArray->SetName("cohesion(Pa)");

	vtkDoubleArray* frictionArray = vtkDoubleArray::New();
	frictionArray->SetName("friction(º)");

	vtkIntArray* excavationYearArray = vtkIntArray::New();
	excavationYearArray->SetName("Excavation Year");

	vtkDoubleArray* damageArray = vtkDoubleArray::New();
	damageArray->SetName("damage");
	
	stateArray->SetNumberOfValues(output->GetNumberOfCells());
	cohesionArray->SetNumberOfValues(output->GetNumberOfCells());
	frictionArray->SetNumberOfValues(output->GetNumberOfCells());
	excavationYearArray->SetNumberOfValues(output->GetNumberOfCells());
	damageArray->SetNumberOfValues(output->GetNumberOfCells());

	for(vtkIdType i=0; i<output->GetNumberOfCells(); ++i)
	{
		stateArray->SetValue(i,0);
		cohesionArray->SetValue(i,0);
		frictionArray->SetValue(i,0);
		excavationYearArray->SetValue(i,0);
		damageArray->SetValue(i,0);
	}

	string line;
	vector<string> lineSplit;

	// skip the headers
	while(!inFile.eof())
	{
		getline(inFile, line);
		if(line.size() > 0)
			break;
	}

	while(!inFile.eof())
	{
		getline(inFile, line);
		if(line.size() < 4)
			continue;

		StringUtilities::split(line, lineSplit, ";");
		vtkIdType id = atoi(lineSplit[0].c_str());
		double state = atof(lineSplit[1].c_str());
		double cohesion = atof(lineSplit[2].c_str());
		double friction = atof(lineSplit[3].c_str());

		int excavationYear = atoi(lineSplit[4].c_str());
		if(lineSplit[4] == "pfin")
			excavationYear = 9999;

		double damage = atof(lineSplit[5].c_str());

		map <vtkIdType, vtkIdType>::iterator it;
		it = cidMap.find(id);
		if(it == cidMap.end())
			throw("zone id not found");
		id = it->second;

		stateArray->SetValue(id,state);
		cohesionArray->SetValue(id,cohesion);
		frictionArray->SetValue(id,friction);
		excavationYearArray->SetValue(id,excavationYear);
		damageArray->SetValue(id,damage);
	}

	inFile.close();

	output->GetCellData()->AddArray(stateArray);
	output->GetCellData()->AddArray(cohesionArray);
	output->GetCellData()->AddArray(frictionArray);
	output->GetCellData()->AddArray(excavationYearArray);
	output->GetCellData()->AddArray(damageArray);

	stateArray->Delete();
	cohesionArray->Delete();
	frictionArray->Delete();
	excavationYearArray->Delete();
	damageArray->Delete();
}


//_________________________________________________________________________
void vtkFLAC3DReader::importTensors(vtkUnstructuredGrid *output)
{
	// Make sure we have a file to read.
	if(!this->ZonesTensorsFile)  {
		return;
	}
	if(strlen(this->ZonesTensorsFile)==0)  {
		return;
	}


	ifstream inFile;
	inFile.open(this->ZonesTensorsFile, ios::in);
	if(!inFile)
	{
		vtkErrorMacro("File Error: cannot open file: "<< this->ZonesTensorsFile);
		return;
	}

	vtkDoubleArray* tensorsArray = vtkDoubleArray::New();
	tensorsArray->SetName("tensors(Pa)");
	tensorsArray->SetNumberOfComponents(9);
	
	tensorsArray->SetNumberOfTuples(output->GetNumberOfCells());
	for(vtkIdType i=0; i<output->GetNumberOfCells(); ++i)
	{
		tensorsArray->SetTuple9(i,0,0,0,0,0,0,0,0,0);
	}

	string line;
	vector<string> lineSplit;

	// skip the headers
	while(!inFile.eof())
	{
		getline(inFile, line);
		if(line.size() > 0)
			break;
	}

	while(!inFile.eof())
	{
		getline(inFile, line);
		if(line.size() < 4)
			continue;

		StringUtilities::split(line, lineSplit, ";");
		vtkIdType id = atoi(lineSplit[0].c_str());
		double sxx = atof(lineSplit[1].c_str());
		double syy = atof(lineSplit[2].c_str());
		double szz = atof(lineSplit[3].c_str());
		double sxy = atof(lineSplit[4].c_str());
		double sxz = atof(lineSplit[5].c_str());
		double syz = atof(lineSplit[6].c_str());

		map <vtkIdType, vtkIdType>::iterator it;
		it = cidMap.find(id);
		if(it == cidMap.end())
			throw("zone id not found");
		id = it->second;

		tensorsArray->SetTuple9(id, sxx, sxy, sxz, sxy, syy, syz, sxz, syz, szz);
	}

	inFile.close();

	output->GetCellData()->AddArray(tensorsArray);
	tensorsArray->Delete();
}