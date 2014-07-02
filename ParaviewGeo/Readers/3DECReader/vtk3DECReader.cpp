#include "vtk3DECReader.h"

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


vtkCxxRevisionMacro(vtk3DECReader, "$Revision: 1.1 $");
vtkStandardNewMacro(vtk3DECReader);

//_________________________________________________________________________
vtk3DECReader::vtk3DECReader()
{
	this->FileName = NULL;
	this->SetNumberOfInputPorts(0);

	this->DisplacementVectorsFile = NULL;
	this->ZonesScalarsFile = NULL;
	this->ZonesTensorsFile = NULL;
	this->PointsFile = NULL;
}

//_________________________________________________________________________
vtk3DECReader::~vtk3DECReader()
{
	this->SetFileName(NULL);
}

//_________________________________________________________________________
void vtk3DECReader::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os,indent);
	os << indent <<  "FileName: "
		<< (this->FileName ? this->FileName : "(none)") << "\n";
}

//_________________________________________________________________________
int vtk3DECReader::CanReadFile( const char* fname )
{
	return 1;	
}

//_________________________________________________________________________
int vtk3DECReader::RequestInformation(vtkInformation* request,
									  vtkInformationVector** inputVector,
									  vtkInformationVector* outputVector)
{
	return 1;
}


//_________________________________________________________________________
int vtk3DECReader::RequestData(vtkInformation* request,
							   vtkInformationVector** inputVector,
							   vtkInformationVector* outputVector)
{
	vtkInformation *outInfo = outputVector->GetInformationObject(0);
	vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

	
	vtkUnstructuredGrid* temp= vtkUnstructuredGrid::New();
	
	pidMap.clear();
	cidMap.clear();
	
	this->importPoints(temp);
	this->importTetrahedras(temp);
	
	output->ShallowCopy(temp);
	temp->Delete();

	this->importDisplacements(output);
	this->importScalars(output);
	this->importTensors(output);

	pidMap.clear();
	cidMap.clear();

	return 1;
}

//_________________________________________________________________________
void vtk3DECReader::importPoints(vtkUnstructuredGrid* output)
{
	// Make sure we have a file to read.
	if(!this->PointsFile)  {
		return;
	}
	if(strlen(this->PointsFile)==0)  {
		return;
	}


	ifstream inFile;
	inFile.open(this->PointsFile, ios::in);
	if(!inFile)
	{
		vtkErrorMacro("File Error: cannot open file: "<< this->PointsFile);
		return;
	}

	vtkPoints* outPoints = vtkPoints::New();
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
		double x = atof(lineSplit[1].c_str());
		double y = atof(lineSplit[2].c_str());
		double z = atof(lineSplit[3].c_str());

		vtkIdType newId = outPoints->InsertNextPoint(x, y, z);
		pidMap[id] = newId;
	}

	output->SetPoints(outPoints);
	outPoints->Delete();
	inFile.close();
}

//_________________________________________________________________________
void vtk3DECReader::importTetrahedras(vtkUnstructuredGrid* output)
{
	// Make sure we have a file to read.
	if(!this->FileName)  {
		return;
	}
	if(strlen(this->FileName)==0)  {
		return;
	}


	ifstream inFile;
	inFile.open(this->FileName, ios::in);
	if(!inFile)
	{
		vtkErrorMacro("File Error: cannot open file: "<< this->FileName);
		return;
	}
	
	vtkIntArray* blkIdsArray = vtkIntArray::New();
	blkIdsArray->SetName("Block Id");
	
	string line;
	vector<string> lineSplit;
	vtkIdType ids[4];

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
		if(line.size() < 6)
			continue;

		StringUtilities::split(line, lineSplit, ";");
		vtkIdType blkId = atoi(lineSplit[0].c_str());
		vtkIdType zoneId = atoi(lineSplit[1].c_str());
		
		ids[0] = atoi(lineSplit[2].c_str());
		ids[1] = atoi(lineSplit[3].c_str());
		ids[2] = atoi(lineSplit[4].c_str());
		ids[3] = atoi(lineSplit[5].c_str());
		
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
		
		vtkIdType newZoneId = output->InsertNextCell(VTK_TETRA,4,ids);
		blkIdsArray->InsertNextValue(blkId);
		
		cidMap[zoneId] = newZoneId;
	}

	inFile.close();
	
	output->GetCellData()->AddArray(blkIdsArray);
	blkIdsArray->Delete();
}

//_________________________________________________________________________
void vtk3DECReader::importDisplacements(vtkUnstructuredGrid *output)
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
			throw("displacement file contain inexisting point");
		id = it->second;		

		dispArray->SetTuple3(id, dispX, dispY, dispZ);
	}

	inFile.close();

	output->GetPointData()->AddArray(dispArray);
	dispArray->Delete();
}

//_________________________________________________________________________
void vtk3DECReader::importScalars(vtkUnstructuredGrid *output)
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

	//vtkIntArray* excavationYearArray = vtkIntArray::New();
	//excavationYearArray->SetName("Excavation Year");

	//vtkDoubleArray* damageArray = vtkDoubleArray::New();
	//damageArray->SetName("damage");
	
	stateArray->SetNumberOfValues(output->GetNumberOfCells());
	cohesionArray->SetNumberOfValues(output->GetNumberOfCells());
	frictionArray->SetNumberOfValues(output->GetNumberOfCells());
	//excavationYearArray->SetNumberOfValues(output->GetNumberOfCells());
	//damageArray->SetNumberOfValues(output->GetNumberOfCells());

	for(vtkIdType i=0; i<output->GetNumberOfCells(); ++i)
	{
		stateArray->SetValue(i,0);
		cohesionArray->SetValue(i,0);
		frictionArray->SetValue(i,0);
		//excavationYearArray->SetValue(i,0);
		//damageArray->SetValue(i,0);
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

		//int excavationYear = atoi(lineSplit[4].c_str());
		//if(lineSplit[4] == "pfin")
		//	excavationYear = 9999;

		//double damage = atof(lineSplit[5].c_str());
		
		map <vtkIdType, vtkIdType>::iterator it;
		it = cidMap.find(id);
		if(it == cidMap.end())
			throw("scalars file contain inexisting zone");
		id = it->second;				

		stateArray->SetValue(id,state);
		cohesionArray->SetValue(id,cohesion);
		frictionArray->SetValue(id,friction);
		//excavationYearArray->SetValue(id,excavationYear);
		//damageArray->SetValue(id,damage);
	}

	inFile.close();

	output->GetCellData()->AddArray(stateArray);
	output->GetCellData()->AddArray(cohesionArray);
	output->GetCellData()->AddArray(frictionArray);
	//output->GetCellData()->AddArray(excavationYearArray);
	//output->GetCellData()->AddArray(damageArray);

	stateArray->Delete();
	cohesionArray->Delete();
	frictionArray->Delete();
	//excavationYearArray->Delete();
	//damageArray->Delete();

}


//_________________________________________________________________________
void vtk3DECReader::importTensors(vtkUnstructuredGrid *output)
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
			throw("tensors file contain inexisting zone");
		id = it->second;	
		
		tensorsArray->SetTuple9(id, sxx, sxy, sxz, sxy, syy, syz, sxz, syz, szz);
	}

	inFile.close();

	output->GetCellData()->AddArray(tensorsArray);
	tensorsArray->Delete();
}