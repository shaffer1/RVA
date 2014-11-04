// ZMapPlus GRID file reader.
#include "ZMapPlusReader.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkObjectFactory.h"
#include "vtkImageData.h"
#include "vtkFloatArray.h"
#include "vtkSmartPointer.h"
#include "vtkPointData.h"

#include <sstream>
#include <limits>


// helper
std::string trim(std::string const&);

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

int ZMapPlusReader::RequestInformation(vtkInformation* vtkNotUsed(request),
                                       vtkInformationVector** vtkNotUsed(inputVector),
                                       vtkInformationVector* outputVector) 
{
    vtkInformation* outInfo = outputVector->GetInformationObject(0);
    double spacing[3], origin[3];
    int extent[6];

    if (!this->FileName)
    {
        vtkErrorMacro(<< "A FileName must be specified.");
        return 0;
    }
    
    this->ReadHeader();
    extent[0] = 0;
    extent[1] = this->NumberOfColumns - 1;
    extent[2] = 0;
    extent[3] = this->NumberOfRows - 1;
    extent[4] = extent[5] = 0;
    
    origin[0] = this->Xmin;
    origin[1] = this->Ymin;
    origin[2] = 0;

    float dx = (this->Xmax - this->Xmin)/float(this->NumberOfColumns - 1);
    float dy = (this->Ymax - this->Ymin)/float(this->NumberOfRows - 1);
    spacing[0] = dx;
    spacing[1] = dy;
    spacing[2] = 0.0f;

    outInfo->Set(vtkDataObject::ORIGIN(), origin, 3);
    outInfo->Set(vtkDataObject::SPACING(), spacing, 3);
    this->GetOutput()->SetNumberOfScalarComponents(1);
    this->GetOutput()->SetScalarType(VTK_FLOAT);
    outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), extent, 6);
    
    return 1;    
}    

int ZMapPlusReader::ReadHeader() {
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
    std::string NullText;
    int Decimals;
    int StartColumn;
    std::string line;
    std::stringstream ss;

    vtkDebugMacro("Reading ZMapPlus file header: " << this->FileName);

    while (true) {
        std::getline(zmapfile, line);
        if (line.size() == 0) {
            break;
        }
        else if (line.at(0) == '!') {
            continue;
        }
        else if (line.at(0) == '@' && line.size() > 1) {
            // The header is comma delimited and there may be an arbitrary amount
            // of whitespace for each item in the list. Also, some items may be
            // blank or missing. 
            std::string token;
            ss.str(line);
            std::getline(ss, token, ',');
            UserText = trim(token);
            std::getline(ss, token, ',');
            ZMapType = trim(token);
            ss >> NumColsPerLine; 
            ss.clear();
            if (ZMapType == "GRID") {
                // Second line is field width, null value, null text, decimals, starting column
                getline(zmapfile, line);
                ss.str(line);
                std::getline(ss, token, ',');
                FieldWidth = atoi(token.c_str());
                std::getline(ss, token, ',');
                NullValue = atof(token.c_str());
                std::getline(ss, token, ',');
                NullText = token;
                std::getline(ss, token, ',');
                Decimals = atoi(token.c_str());
                ss >> StartColumn;
                ss.clear();

                // Third line is number of rows, number of cols, x min, x max, y min, y max
                getline(zmapfile, line);
                ss.str(line);
                std::getline(ss, token, ',');
                NumberOfRows = atoi(token.c_str());
                std::getline(ss, token, ',');
                NumberOfColumns = atoi(token.c_str());
                std::getline(ss, token, ',');
                Xmin = atof(token.c_str());
                std::getline(ss, token, ',');
                Xmax = atof(token.c_str());
                std::getline(ss, token, ',');
                Ymin = atof(token.c_str());
                std::getline(ss, token, ',');
                Ymax = atof(token.c_str());
                ss.clear();

                // Fourth line is irrelevant line of zeros
                getline(zmapfile, line);
                // Fifth line is just '@'
                getline(zmapfile, line);
                pos = zmapfile.tellg();
            }
            else {
                vtkErrorMacro("Not a GRID file, type is: " << ZMapType);
                return 0;
            }
        }
        return 1;
    }
    // possible empty header or unexpected newline.
    return 0;
}

int ZMapPlusReader::RequestData(vtkInformation* vtkNotUsed(request),
                                vtkInformationVector** vtkNotUsed(inputVector),
                                vtkInformationVector* outputVector)
{
    vtkInformation *outInfo = outputVector->GetInformationObject(0);
    vtkImageData* output = vtkImageData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
   
    output->SetExtent(outInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT()));    
    output->AllocateScalars();

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
    
    // skip header, header ends with a single '@' line.    
    zmapfile.seekg(pos);
    std::string line;
    std::stringstream ss;
    std::vector<float> data;
    while (true) {
        std::getline(zmapfile, line);
        if (line.size() == 0) {
            break;
        }
        // The data section is space delimited (or technically FieldWidth
        // num chars, so fields could abut.)
        //std::cout << "MVM: line: " << line << std::endl; 
        ss.str(line);
        float tmp;
        while (ss >> tmp) {
            data.push_back(tmp);
        } 
        ss.clear();
    }
    vtkDebugMacro("Finished reading ZMapPlus file");

        
    vtkSmartPointer<vtkFloatArray> heights = vtkSmartPointer<vtkFloatArray>::New();
    int i = 0;
    for (int col = 0; col < NumberOfColumns; ++col) {
        for (int row = NumberOfRows - 1; row >=0; --row) {
            int index = col + NumberOfColumns * row;
            if (data.at(i) == NullValue) {
                data.at(i) = std::numeric_limits<float>::quiet_NaN();
            }
            heights->InsertTuple1(index, data.at(i));
            i++;
        }
    }

    output->GetPointData()->SetScalars(heights);
    output->GetPointData()->GetScalars()->SetName("Elevation");

    return 1;
}

std::string trim(std::string const& s) {
    // trims whitespace from before and after string.
    std::size_t start = s.find_first_not_of(' ');
    std::size_t end = s.find_last_not_of(' ');
    return s.substr(start, start - end + 1);
}
