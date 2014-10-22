// ZMapPlus GRID file reader.

#include <sstream>
#include <limits>

#include "ZMapPlusReader.h"
#include "vtkImageData.h"
#include "vtkFloatArray.h"
#include "vtkSmartPointer.h"
#include "vtkPointData.h"

vtkStandardNewMacro(ZMapPlusReader);

ZMapPlusReader::ZMapPlusReader()
{
    this->FileName = 0;
    this->SetNumberOfInputPorts(0);
}

ZMapPlusReader::~ZMapPlusReader()
{
    this->SetFileName(0);
}

void ZMapPlusReader::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os,indent);
    os << indent << "FileName: " 
        << (this->FileName ? this->FileName : "(none)") << "\n";
}

int ZMapPlusReader::RequestData(vtkInformation*,
                                vtkInformationVector**,
                                vtkInformationVector* outputVector)
{
    if (!this->FileName)
    {
        vtkErrorMacro("A FileName must be specified.");
        return 0;
    }

    ifstream zmapfile(this->FileName);
    if (!zmapfile)
    {
        vtkErrorMacro("Error opening file " << this->FileName);
        return 0;
    }

    std::string UserText;
    std::string ZMapType;
    int NumColsPerLine;
    int FieldWidth;
    float NullValue;
    std::string NullText;
    int Decimals;
    int StartColumn;
    int NumberOfRows;
    int NumberOfColumns;
    float Xmin;
    float Xmax;
    float Ymin;
    float Ymax;
    std::vector<float> data;
    std::string line;
    std::stringstream ss;

    vtkDebugMacro("Reading ZMapPlus file: " << this->FileName);

    while (true) {
        zmapfile.getline(line);
        if (!line) {
            break;
        }
        else if (line.at(0) == '!') {
            continue;
        }
        else if (line.at(0) == '@' and line.size() > 1) {
            // The header is comma delimited
            sscanf(line.c_str(), "@%s, %s, %d", &UserText[0], &ZMapType[0], &NumColsPerLine);
            if (ZMapType == "GRID") {
                zmapfile.getline(line);
                sscanf(line.c_str(), "%d, %d, %f, %s, %d, %d", &FieldWidth, 
                                                               &NullValue, 
                                                               &NullText[0], 
                                                               &Decimals, 
                                                               &StartColumn); 
                zmapfile.getline(line);
                sscanf(line.c_str(), "%d, %d, %f, %f, %f, %f", &NumberOfRows,
                                                               &NumberOfColumns,
                                                               &Xmin,
                                                               &Xmax,
                                                               &Ymin,
                                                               &Ymax);
                // irrelevant line of zeros
                zmapfile.getline(line);
                // line of just '@'
                zmapfile.getline(line);
            }
            else {
                vtkErrorMacro("Not a GRID file, type is: " << ZMapType);
                return 0;
            }
        }
        else {
            // The data section is space delimited (or technically FieldWidth
            // num chars, so fields could abut.) 
            ss.str(line);
            float tmp;
            while (ss >> tmp) {
               data.push_back(tmp);
            } 
            ss.clear();
        }
    }
    vtkDebugMacro("Finished reading ZMapPlus file");

    vtkImageData* output = vtkImageData::GetData(outputVector);
    output->SetExtent(0, NumberOfColumns - 1, 0, NumberOfRows - 1, 0, 0);
    output->SetOrigin(Xmin, Ymin, 0);
    float dx = (Xmax - Xmin)/float(NumberOfColumns - 1);
    float dy = (Ymax - Ymin)/float(NumberOfRows - 1);
    output->SetSpacing(dx, dy, 0);

    vtkSmartPointer<vtkFloatArray> heights = vtkSmartPointer<vtkFloatArray>::New();
    int i = 0;
    for (int col = 0; col < NumberOfColumns; ++col) {
        for (int row = NumberOfRows - 1; row >=0; --row) {
            int index = col + NumberOfColumns * row;
            if (data.at(i) == NullValue) {
                data.at(i) = std::numeric_limits<float>::quiet_NaN()
            }
            heights->InsertTuple1(index, data.at(i));
            i++;
        }
    }

    output->GetPointData()->SetScalars(heights);

    return 1;
}
    
