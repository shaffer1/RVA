#ifndef __Volumetrics_h
#define __Volumetrics_h

#include "vtkAlgorithm.h"
#include "vtkArrayCalculator.h"
#include "vtkIntegrateAttributes.h"
#include "vtkSmartPointer.h"

class Volumetrics : public vtkUnstructuredGridAlgorithm
{
    public:
        vtkTypeMacro(Volumetrics,vtkAlgorithm);
        void PrintSelf(ostream& os, vtkIndent indent);

        static Volumetrics *New();

        vtkGetMacro(multiplier, std::string);
        vtkSetMacro(multiplier, std::string);

        vtkGetMacro(multiplicand, std::string);
        vtkSetMacro(multiplicand, std::string);

        vtkGetMacro(ResultArrayName, std::string);
        vtkSetMacro(ResultArrayName, std::string);

    protected:
        Volumetrics();
        ~Volumetrics();

        int RequestData(vtkInformation *, vtkInformationVector **,
                vtkInformationVector *);

        // Overriding in order to accept non-unstructured grid inputs.
        virtual int FillInputPortInformation(int, vtkInformation*);

    private:
        Volumetrics(const Volumetrics&); // Not implemented.
        void operator=(const Volumetrics&); // Not implemented.

        vtkSmartPointer<vtkArrayCalculator> calc;
        vtkSmartPointer<vtkIntegrateAttributes> integrate;

        // The scalars multiplieed
        std::string multiplier;
        std::string multiplicand;
        std::string ResultArrayName;
};

#endif
