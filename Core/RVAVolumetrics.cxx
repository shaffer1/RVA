#include "RVAVolumetrics.h"

#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkInformationVector.h"
#include "vtkInformation.h"
#include "vtkDataObject.h"
#include "vtkDataSet.h"
#include "vtkUnstructuredGrid.h"
#include "vtkCellData.h"
#include "vtkDoubleArray.h"
#include "vtkVertex.h"
#include "vtkPoints.h"

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
    vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

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
    this->integrate->SetInput(input);
    this->integrate->Update();
    vtkStdString scalarName;
    scalarName = this->integrate->GetOutput()->GetCellData()->GetArrayName(0);
    this->integrate->GetOutput()->GetCellData()->SetActiveScalars(scalarName.c_str());

    double dummy = 0.12345;

    // The Integrate Variables filter creates a data set of a single point on which
    // to attached the field data output. This does the same.
    vtkSmartPointer<vtkPoints> pt = vtkSmartPointer<vtkPoints>::New();
    pt->InsertNextPoint(0.0, 0.0, 0.0);
    vtkSmartPointer<vtkVertex> vtx = vtkSmartPointer<vtkVertex>::New();
    vtx->GetPointIds()->InsertNextId(0);
    vtkSmartPointer<vtkDoubleArray> results = vtkSmartPointer<vtkDoubleArray>::New();
    results->SetName("Results");
    results->InsertNextTuple(&dummy);
   
    output->Allocate(1);
    output->InsertNextCell(vtx->GetCellType(), vtx->GetPointIds());
    output->SetPoints(pt);
    output->GetCellData()->SetScalars(results);

    return 1;
}

void RVAVolumetrics::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os, indent);
}
