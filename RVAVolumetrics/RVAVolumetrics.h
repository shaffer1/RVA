#ifndef __RVAVolumetrics_h
#define __RVAVolumetrics_h

#include "vtkAlgorithm.h"
#include "vtkArrayCalculator.h"
#include "vtkIntegrateAttributes.h"
#include "vtkSmartPointer.h"

class RVAVolumetrics : public vtkUnstructuredGridAlgorithm
{
    public:
        vtkTypeMacro(RVAVolumetrics,vtkAlgorithm);
        void PrintSelf(ostream& os, vtkIndent indent);

        static RVAVolumetrics *New();

        vtkGetMacro(multiplier, std::string);
        vtkSetMacro(multiplier, std::string);

        vtkGetMacro(multiplicand, std::string);
        vtkSetMacro(multiplicand, std::string);

        vtkGetMacro(ResultArrayName, std::string);
        vtkSetMacro(ResultArrayName, std::string);

    protected:
        RVAVolumetrics();
        ~RVAVolumetrics();

        int RequestData(vtkInformation *, vtkInformationVector **,
                vtkInformationVector *);

        // Overriding in order to accept non-unstructured grid inputs.
        virtual int FillInputPortInformation(int, vtkInformation*);

    private:
        RVAVolumetrics(const RVAVolumetrics&); // Not implemented.
        void operator=(const RVAVolumetrics&); // Not implemented.

        vtkSmartPointer<vtkArrayCalculator> calc;
        vtkSmartPointer<vtkIntegrateAttributes> integrate;

        // The scalars multiplieed
        std::string multiplier;
        std::string multiplicand;
        std::string ResultArrayName;
};

#endif