

#include "vtkFractureTrace.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkStructuredGrid.h"
#include "vtkCellArray.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkMath.h"
#include "vtkPlane.h"
#include "vtkCylinderSource.h"
#include "vtkConeSource.h"
#include "vtkTransformFilter.h"
#include "vtkTransform.h"
#include "vtkLineSource.h"
#include "vtkCellData.h"
#include "vtkLine.h"

vtkCxxRevisionMacro(vtkFractureTrace, "$Revision: 1.0 $");
vtkStandardNewMacro(vtkFractureTrace);

vtkFractureTrace::vtkFractureTrace()
{
}

vtkFractureTrace::~vtkFractureTrace()
{
}


int vtkFractureTrace::RequestData(vtkInformation *request, 
								  vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
	// get the info objects
	vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
	vtkInformation *outInfo = outputVector->GetInformationObject(0);

	// get the input and ouptut
	this->input = vtkPolyData::SafeDownCast(
		inInfo->Get(vtkDataObject::DATA_OBJECT()));
	this->output = vtkPolyData::SafeDownCast(
		outInfo->Get(vtkDataObject::DATA_OBJECT()));

	vtkCellArray* inCells = this->input->GetPolys();
	inCells->InitTraversal();
	vtkIdType nCells = inCells->GetNumberOfCells();

	inPoints = input->GetPoints();
	vtkIdType nPoints = inPoints->GetNumberOfPoints();

	this->append = vtkAppendPolyData::New();

	vtkIntArray* isNormal = vtkIntArray::New();
	isNormal->SetName("Normal(0)_Dip(1)_Intersection(2)");

	vtkIdType npts;
	vtkIdType* pts;
	for(vtkIdType i=0; i<nCells; ++i)
	{
		inCells->GetNextCell(npts, pts);
		if(npts < 3)
		{
			vtkWarningMacro("The cells should at least have three points");
			continue;
		}
		double pt1[3], pt2[3], pt3[3];
		inPoints->GetPoint(pts[0], pt1);
		inPoints->GetPoint(pts[1], pt2);
		inPoints->GetPoint(pts[2], pt3);

		double v1[] = {pt2[0]-pt1[0], pt2[1]-pt1[1], pt2[2]-pt1[2]};
		double v2[] = {pt3[0]-pt1[0], pt3[1]-pt1[1], pt3[2]-pt1[2]};
		double N[3];
		vtkMath::Cross(v1, v2, N);
		vtkMath::Normalize(N);

		double center[3];
		this->computeFractureCenter(npts, pts, center);
		double length = this->computeNormalLength(npts, pts);

		// projection of [0,0,-1] on the plane with normal V
		double dip[] = {N[0]*N[2], N[1]*N[2], -1.0 + N[2]*N[2]};
		vtkMath::Normalize(dip);

		int n = this->addArrow(center, N, length, 0);
		//for(int count=0; count<n; ++count) isNormal->InsertNextValue(0);

		n = this->addArrow(center, dip, length, 1);
		//for(int count=0; count<n; ++count) isNormal->InsertNextValue(1);

		vtkIdType traversal = inCells->GetTraversalLocation();
		for(vtkIdType j=i+1; j<nCells; ++j)
		{
			vtkIdType npts2;
			vtkIdType* pts2;
			inCells->GetNextCell(npts2, pts2);
			if(npts2 < 3)
				continue;

			double p1[3], p2[3], p3[3];
			inPoints->GetPoint(pts2[0], p1);
			inPoints->GetPoint(pts2[1], p2);
			inPoints->GetPoint(pts2[2], p3);

			double v1[] = {p2[0]-p1[0], p2[1]-p1[1], p2[2]-p1[2]};
			double v2[] = {p3[0]-p1[0], p3[1]-p1[1], p3[2]-p1[2]};
			double N2[3];
			vtkMath::Cross(v1, v2, N2);
			vtkMath::Normalize(N2);

			double V[3];
			vtkMath::Cross(N, N2, V); // the direction of the line
			double norm = vtkMath::Normalize(V);

			// check if the two fracture are parallel
			if(norm == 0)
				continue;

			// perform bounding box intersection first

			double s1, s2, a, b;

			// ax + by + cz + s = 0
			s1 = (N[0]*pt1[0] + N[1]*pt1[1] + N[2]*pt1[2]);
			s2 = (N2[0]*p1[0] + N2[1]*p1[1] + N2[2]*p1[2]);
			
			double n1n2dot = vtkMath::Dot(N, N2);
			double n1normsqr = 1.;//vtkMath::Dot(N,N);
			double n2normsqr = 1.;//vtkMath::Dot(N2, N2);

			a = (s2 * n1n2dot - s1 * n2normsqr) / (n1n2dot*n1n2dot - n1normsqr * n2normsqr);
			b = (s1 * n1n2dot - s2 * n2normsqr) / (n1n2dot*n1n2dot - n1normsqr * n2normsqr);

			double lPoint[3]; // the point on the line
			lPoint[0] = a*N[0] + b*N2[0];
			lPoint[1] = a*N[1] + b*N2[1];
			lPoint[2] = a*N[2] + b*N2[2];

			double lPoint2[] = {lPoint[0]+V[0], lPoint[1]+V[1], lPoint[2]+V[2]};

			double t1, t2, t3, t4;
			if(!this->intersectWithCell(lPoint, V, npts, pts, t1, t2)) continue;
			if(!this->intersectWithCell(lPoint, V, npts2, pts2, t3, t4)) continue;
			
			double tLeft = std::max(t1, t3);
			double tRight = std::min(t2, t4);
			if(tLeft >= tRight)
				continue;

			p1[0] = lPoint[0] + V[0]*tLeft;
			p1[1] = lPoint[1] + V[1]*tLeft;
			p1[2] = lPoint[2] + V[2]*tLeft;

			p2[0] = lPoint[0] + V[0]*tRight;
			p2[1] = lPoint[1] + V[1]*tRight;
			p2[2] = lPoint[2] + V[2]*tRight;

			vtkLineSource *line = vtkLineSource::New();
			line->SetPoint1(p1);
			line->SetPoint2(p2);
			line->SetResolution(2);
			line->Update();
			vtkIntArray* props = vtkIntArray::New();
			props->SetName("Normal(0)_Dip(1)_Intersection(2)");
			props->InsertNextValue(2);
			line->GetOutput()->GetCellData()->AddArray(props);
			append->AddInput(line->GetOutput());
			line->Delete();
		}
		inCells->SetTraversalLocation(traversal);
	}

	this->append->Update();
	this->output->DeepCopy(append->GetOutput());
	this->append->Delete();
	return 1;
}

//----------------------------------------------------------------------------
bool vtkFractureTrace::intersectWithCell(const double *pt, const double *V, 
										 	vtkIdType npts, const vtkIdType* pts, double &t1, double &t2)
{
	int count = 0;
	double T[2];
	for(vtkIdType i=0; i<npts-1; ++i)
	{
		double p1[3], p2[3];
		this->inPoints->GetPoint(pts[i], p1);
		this->inPoints->GetPoint(pts[i+1], p2);

		bool intersects = this->line_segIntersection(pt, V, p1, p2, T[count]);
		if(intersects)
			++count;

		if(count == 2)
			break;
	}
	if(count < 2)
		return false;

	if(T[0] < T[1])
	{
		t1 = T[0];
		t2 = T[1];
	}
	else
	{
		t1 = T[1];
		t2 = T[0];
	}

	return true;
}

//----------------------------------------------------------------------------
bool vtkFractureTrace::line_segIntersection(const double *pt, const double *V, 
											const double *p1, const double *p2, double &t)
{
	double V2[] = {p2[0]-p1[0], p2[1]-p1[1], p2[2]-p1[2]};

	double a0 = V[0], b0 = V[1], c0 = V[2], a1 = V2[0], b1 = V2[1], c1 = V2[2];
	double x0 = pt[0], y0 = pt[1], z0 = pt[2], x1 = p1[0], y1 = p1[1], z1 = p1[2];

	double det = -a0*b1 + a1*b0;
	if(fabs(det) < 1e-3)
		return false;

	double t_seg_int = (a0*(y1-y0)-b0*(x1-x0))/det;
	if(t_seg_int < 0 || t_seg_int > 1)
		return false;

	t = (-b1*(x1-x0) + a1*(y1-y0))/det;
	
	return true;
}

//----------------------------------------------------------------------------
int vtkFractureTrace::addArrow( const double* center, const double* V, double length, int prop)
{
	int n = 0;

	//vtkLineSource *line = vtkLineSource::New();
	//line->SetPoint1((double*)(center));
	//double pt2[] = {center[0] + (length*V[0]), center[1] + (length*V[1]), center[2] + (length*V[2])};
	//line->SetPoint2(pt2);
	//line->Update();
	//append->AddInput(line->GetOutput());


	double c[3];
	c[0] = center[0] + length*V[0]/3;
	c[1] = center[1] + length*V[1]/3;
	c[2] = center[2] + length*V[2]/3;

	//set up the transform for the cylinder
	double rotation[16];
	this->createMatrix(V, c, rotation);
	vtkTransform *transform = vtkTransform::New();
	transform->SetMatrix( rotation );

	//cylinder that is the base of the
	vtkCylinderSource *cylinder = vtkCylinderSource::New();
	cylinder->SetRadius( 0.03 );
	cylinder->SetHeight( 2.*length/3  );
	cylinder->SetCenter( 0, 0, 0 );
	cylinder->Update();

	n += ( cylinder->GetOutput()->GetNumberOfCells() );

	//move the cylinder
	vtkTransformFilter *tf = vtkTransformFilter::New();
	tf->SetTransform( transform );
	tf->SetInput( cylinder->GetOutput() );
	tf->Update();

	//here is the cone that composes the middle section of the line
	vtkConeSource *midCone = vtkConeSource::New();
	midCone->SetHeight(length/3);
	midCone->SetRadius( 0.1);
	midCone->SetCenter( center[0] + (5./6)*length*V[0], 
						center[1] + (5./6)*length*V[1], 
						center[2] + (5./6)*length*V[2] );
	double dir[] = {V[0], V[1], V[2]};
	midCone->SetDirection( dir );
	midCone->SetResolution( 6 );
	midCone->Update();

	vtkPolyData* cone = midCone->GetOutput();
	n = cone->GetNumberOfCells();
	vtkIntArray* props = vtkIntArray::New();
	props->SetName("Normal(0)_Dip(1)_Intersection(2)");
	for(int i=0; i<n;++i) props->InsertNextValue(prop);
	cone->GetCellData()->AddArray(props);

	append->AddInput( cone );
	midCone->Delete();

	vtkPolyData* cylindre = (vtkPolyData*) tf->GetOutput();
	n = cylindre->GetNumberOfCells();
	props = vtkIntArray::New();
	props->SetName("Normal(0)_Dip(1)_Intersection(2)");
	for(int i=0; i<n;++i) props->InsertNextValue(prop);
	cylindre->GetCellData()->AddArray(props);

	append->AddInput( cylindre );

	//clean up the memory we have used
	tf->Delete();
	transform->Delete();
	cylinder->Delete();

	return n;
}

//----------------------------------------------------------------------------
void vtkFractureTrace::createMatrix( const double *direction, const double *center, double matrix[16] )
{

  //storage for cross products
  double norm[3]={0,1,0};
  double firstVector[3];
  double secondVector[3];
  double thirdVector[3];

  //copy the point, so that we do not destroy it
  firstVector[0] = direction[0];
  firstVector[1] = direction[1];
  firstVector[2] = direction[2];
  vtkMath::Normalize( firstVector );

  //have to find the other 2 vectors, to create a proper transform matrix
  vtkMath::Cross( firstVector, norm, secondVector );
  vtkMath::Cross( firstVector, secondVector, thirdVector );

  //rotate and centre according to normalized axes and centre point
  //column 1
  matrix[0] = secondVector[0];
  matrix[1] = firstVector[0];
  matrix[2] = thirdVector[0];
  matrix[3] = center[0];

  //column 2
  matrix[4] = secondVector[1];
  matrix[5] = firstVector[1];
  matrix[6] = thirdVector[1];
  matrix[7] = center[1];

  //column 3
  matrix[8] = secondVector[2];
  matrix[9] = firstVector[2];
  matrix[10] = thirdVector[2];
  matrix[11] = center[2];

  //column 4
  matrix[12] = 0.0;
  matrix[13] = 0.0;
  matrix[14] = 0.0;
  matrix[15] = 1.0;

}

//----------------------------------------------------------------------------
void vtkFractureTrace::computeFractureCenter(vtkIdType npts, const vtkIdType *pts, double *center)
{
	center[0]=0.0; center[1]=0.0; center[2]=0.0;
	double pt[3];
	for(vtkIdType i=0; i<npts; ++i)
	{
		this->inPoints->GetPoint(pts[i], pt);
		center[0] += pt[0];
		center[1] += pt[1];
		center[2] += pt[2];
	}
	center[0] /= npts;
	center[1] /= npts;
	center[2] /= npts;
}

//----------------------------------------------------------------------------
double vtkFractureTrace::computeNormalLength(vtkIdType npts, const vtkIdType *pts)
{
	double length = 0;
	double pt1[3], pt2[3];
	for(vtkIdType i=0; i<npts; ++i)
	{
		this->inPoints->GetPoint(pts[i], pt2);
		for(vtkIdType j=0; j<npts; ++j)
		{
			if(i == j)
				continue;
			this->inPoints->GetPoint(pts[j], pt1);
			double L = vtkMath::Distance2BetweenPoints(pt1, pt2);
			if(L > length)
				length = L;
		}
	}
	return sqrt(length);
}


//----------------------------------------------------------------------------
int vtkFractureTrace::FillInputPortInformation(int, vtkInformation *info)
{
	info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
	return 1;
}