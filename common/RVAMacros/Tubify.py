''' Tubify by Nemo '''
from paraview.simple import *
def Tubify():
    obj = GetActiveSource()


    tu = Tube(obj)

    bds = obj.GetDataInformation().GetBounds()
    deltaX = abs(bds[1] - bds[0])
    deltaY = abs(bds[3] - bds[2])
    deltaZ = abs(bds[5] - bds[4])
    deltaMax = max(deltaX, deltaY, deltaZ)
    tu.Radius = 0.0008 * deltaMax

    dNTu = tu.Scalars.GetArrayName()
    dpTu = GetDisplayProperties(tu)
    dpTu.LookupTable = GetLookupTableForArray(dNTu, 1)
    dpTu.ColorAttributeType = 'POINT_DATA'
    dpTu.ColorArrayName = dNTu


    gl = Glyph(obj)

    gl.SetScaleFactor = 0.35
    gl.ScaleMode = 'off'
    gl.Vectors = ['POINTS', 'Gradients']
    gl.MaskPoints = 0

    dNGl = gl.Scalars.GetArrayName()
    dpGl = GetDisplayProperties(gl)
    dpGl.LookupTable = GetLookupTableForArray(dNGl, 1)
    dpGl.ColorAttributeType = 'POINT_DATA'
    dpGl.ColorArrayName = dNGl


    Show(tu)
    Show(gl)
    Hide(obj)

    Render()
	
