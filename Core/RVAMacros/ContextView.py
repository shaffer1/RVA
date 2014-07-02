try: paraview.simple
except: from paraview.simple import *
paraview.simple._DisableFirstRenderCameraReset()


FocusView = GetRenderView()

ContextView = CreateRenderView()
ContextView.CompressorConfig = 'vtkSquirtCompressor 0 3'
ContextView.UseLight = 1
ContextView.LightSwitch = 0
ContextView.LODThreshold = 5.0
ContextView.LODResolution = 50.0
ContextView.Background = [0.31999694819562063, 0.3400015259021897, 0.4299992370489052]

FocusObject = GetActiveSource()
ObjectRepresentation = GetDisplayProperties(FocusObject)
ObjectRepresentation.Representation = 'Wireframe'

bounds = FocusObject.GetDataInformation().GetBounds()
scaleFactor = (bounds[1]-bounds[0])/20

"""
CrossMarker = a2DGlyph()
CrossMarker.GlyphType = 'Cross'
CrossMarker.Center = FocusView.CameraPosition
CrossRepresentation = GetDisplayProperties(CrossMarker)
CrossRepresentation.DiffuseColor = [1.0, 0.0, 1.0]
"""

SphereMarker = Sphere()
SphereMarker.Center = FocusView.CameraPosition
SphereMarker.Radius = scaleFactor
SphereRepresentation = GetDisplayProperties(SphereMarker)
SphereRepresentation.DiffuseColor = [1.0, 0.0, 1.0]

DataRepresentation4 = Show()

ContextView.CameraPosition = [0,0,0]
ContextView.CameraFocalPoint = [0,0,1]
ContextView.ResetCamera()

Render()
