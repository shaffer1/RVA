#include "RVARVAVolumetrics.h"

#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkInformationVector.h"
#include "vtkInformation.h"
#include "vtkDataObject.h"
#include "vtkDataSet.h"
#include "vtkUnstructuredGrid.h"
#include "vtkCellData.h"
#include "vtkDoubleArray.h"


vtkStandardNewMacro(RVAVolumetrics);

RVAVolumetrics::RVAVolumetrics()
{
    this->SetNumberOfInputPorts(1);
    this->SetNumberOfOutputPorts(1);
    this->calc = vtkSmartPointer<vtkArrayCalculator>::New();
    this->integrate = vtkSmartPointer<vtkIntegrateAttributes>::New();
}

RVAVolumetrics::~RVAVolumetrics()
{
}

int RVAVolumetrics::FillInputPortInformation(int port, vtkInformation* info)
{
    if (!this->Superclass::FillInputPortInformation(port, info))
    {
        return 0;
    }

    if (port == 0)
    {
        info->Set(vtkAlgorithm::INPUT_IS_REPEATABLE(), 1);
        info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataObject");
        return 1;
    }

    vtkErrorMacro("This filter does not have more than 1 input port!");
    return 0;
}

int RVAVolumetrics::RequestData(vtkInformation *vtkNotUsed(request),
                                vtkInformationVector **inputVector,
                                vtkInformationVector *outputVector)
{
    vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
    vtkInformation *outInfo = outputVector->GetInformationObject(0);

    vtkDataSet *input = vtkDataSet::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
    vtkDataSet *output = vtkDataSet::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

    if (!output)
    {
        return 0;
    }

    // What I want
    // User to be able to select two scalars from dropdown menus, then
    // this filter will multiply them and integrate them.

    // Constructing this backwards.
    // Step 1. just get integrate attributes working
    // Step 2. get calculator working with hard coded arrays
    // Step 3. user selectable arrays.
    this->integrate->SetInputData(input);
    this->integrate->Update();
    
    output->ShallowCopy(input);
    return 1;
}

void RVAVolumetrics::PrintSelf(ostream& os, vtkIndent indent)
{
    this->SuperClass::PrintSelf(os, indent);
}
