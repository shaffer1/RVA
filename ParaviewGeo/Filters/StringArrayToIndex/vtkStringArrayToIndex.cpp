#include "vtkStringArrayToIndex.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkInformation.h"
#include "vtkStringArray.h"
#include "vtkPointData.h"
#include "vtkCellData.h"

#include <map>
#include <string>
#include <algorithm>

vtkCxxRevisionMacro(vtkStringArrayToIndex, "$Revision: 1.0 $");
vtkStandardNewMacro(vtkStringArrayToIndex);

//----------------------------------------------------------------------------
vtkStringArrayToIndex::vtkStringArrayToIndex()
{
	this->StrProp = NULL;
	this->IndexName = NULL;
}

//----------------------------------------------------------------------------
vtkStringArrayToIndex::~vtkStringArrayToIndex()
{
	this->SetStrProp(NULL);
	this->SetIndexName(NULL);
}

//----------------------------------------------------------------------------
int vtkStringArrayToIndex::RequestData(vtkInformation *request, 
								  vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
	// get the info objects
	vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
	vtkInformation *outInfo = outputVector->GetInformationObject(0);

	// get the input and ouptut
	this->input = vtkDataSet::SafeDownCast(
		inInfo->Get(vtkDataObject::DATA_OBJECT()));
	this->output = vtkDataSet::SafeDownCast(
		outInfo->Get(vtkDataObject::DATA_OBJECT()));
	
	// First, assume data is Point data
	std::string type = "Point";
	vtkStringArray *properties;
	properties = vtkStringArray::SafeDownCast(
				this->input->GetPointData()->GetAbstractArray(this->StrProp));

	// If not point, then it's Cell data
	if (properties == NULL)
	{
		properties = vtkStringArray::SafeDownCast(
				this->input->GetCellData()->GetAbstractArray(this->StrProp));
		type = "Cell";
	}

	if (properties == NULL)
	{
		vtkErrorMacro("The selected data array is invalid: Probably not a string type.");
		return 1;
	}

	// check that the given name doesn't exist in the property arrays
	std::string iname = this->IndexName;
	std::transform(iname.begin(), iname.end(), iname.begin(), ::tolower);

	if (type == "Cell")
	{
		for (int i = 0; i < this->input->GetCellData()->GetNumberOfArrays(); i++)
		{
			std::string name = this->input->GetCellData()->GetArrayName(i);
			std::transform(name.begin(), name.end(), name.begin(), ::tolower);
			if (name == iname)
			{
				vtkErrorMacro("Name already exists in properties. Please select a unique name.");
				return 1;
			}
		}
	}
	else
	{
		for (int i = 0; i < this->input->GetPointData()->GetNumberOfArrays(); i++)
		{
			std::string name = this->input->GetPointData()->GetArrayName(i);
			std::transform(name.begin(), name.end(), name.begin(), ::tolower);
			if (name == iname)
			{
				vtkErrorMacro("Name already exists in properties. Please select a unique name.");
				return 1;
			}
		}
	}

	vtkIntArray* indexArray = vtkIntArray::New();
	indexArray->SetName(this->IndexName);

	// Create unique indicies from 0 -> n
	int uniqueIndex = 0;
	std::map<std::string, int> uniqueIndicies;
	for (int i = 0; i < properties->GetNumberOfValues(); i++)
	{
		int value;
		std::map<std::string, int>::iterator finder;
		finder =  uniqueIndicies.find(properties->GetValue(i).c_str());
		// Check if the property has been found already
		if (finder == uniqueIndicies.end())
		{
			uniqueIndicies[properties->GetValue(i).c_str()] = uniqueIndex;
			value = uniqueIndex;
			uniqueIndex++;
		}
		else
			value = finder->second;

		indexArray->InsertNextValue(value);
	} 

	this->output->ShallowCopy(this->input);
	output->GetPointData()->PassData ( input->GetPointData() );
	output->GetCellData()->PassData ( input->GetCellData() );
	if (type == "Point")
		this->output->GetPointData()->AddArray(indexArray);
	else // Cell
		this->output->GetCellData()->AddArray(indexArray);

	indexArray->Delete();
	return 1;
}

//----------------------------------------------------------------------------
int vtkStringArrayToIndex::FillInputPortInformation(int, vtkInformation *info)
{
	info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
	return 1;
}