# coding: utf-8
# Alithia Model (ALM) exporter for Blender
#

import struct
import bpy

__author__ = "Kostas Michalopoulos"
__version__ = "0.1"
__bpydoc__ = """\
This is an exporter for ALithia Model (ALM) files.
"""

class AlithiaModelExporter:
        verts = []
        faces = []
        
        def add_vert(self, x, y, z, nx, ny, nz, s, t):
            idx = 0
            for v in self.verts:
                if (v == (x, y, z, nx, ny, nz, s, t)):
                    return idx
                idx = idx + 1
            self.verts.append((x, y, z, nx, ny, nz, s, t))
            return len(self.verts) - 1
        
        def add_face(self, x1, y1, z1, nx1, ny1, nz1, s1, t1, x2, y2, z2, nx2, ny2, nz2, s2, t2, x3, y3, z3, nx3, ny3, nz3, s3, t3):
            a = self.add_vert(x1, y1, z1, nx1, ny1, nz1, s1, t1)
            b = self.add_vert(x2, y2, z2, nx2, ny2, nz2, s2, t2)
            c = self.add_vert(x3, y3, z3, nx3, ny3, nz3, s3, t3)
            self.faces.append((a, b, c))
        
        def write(self, filename, scene, ob, scale, frame_start, frame_end, EXPORT_APPLY_MODIFIERS = True):
                if not filename.lower().endswith('.alm'):
                        filename += '.alm'
        
                if not ob:
                        raise Exception("Error, select an object first to export")
                        return

                if EXPORT_APPLY_MODIFIERS:
                        mesh = ob.create_mesh(scene, True, 'PREVIEW')
                else:
                        mesh = ob.data.copy()
                if not mesh:
                        raise("Error, could not get mesh data from the active object")
                        return
                        
                uv_layer = None
                for lay in mesh.uv_textures:
                        if lay.active:
                                uv_layer = lay.data
                                break
        
                if not uv_layer:
                        raise("Error, no active face UV layer")
                        return
                
                self.verts = []
                self.faces = []
        
                for i, f in enumerate(mesh.faces):
                    verts = f.verts
                    uv = uv_layer[i]
                    uv = uv.uv1, uv.uv2, uv.uv3, uv.uv4    
                    if len(verts) == 3:
                        a = mesh.verts[verts[0]]
                        b = mesh.verts[verts[1]]
                        c = mesh.verts[verts[2]]
                        
                        self.add_face(a.co[0]*scale, a.co[1]*scale, a.co[2]*scale, a.normal[0], a.normal[1], a.normal[2], uv[0][0], uv[0][1], b.co[0]*scale, b.co[1]*scale, b.co[2]*scale, b.normal[0], b.normal[1], b.normal[2], uv[1][0], uv[1][1], c.co[0]*scale, c.co[1]*scale, c.co[2]*scale, c.normal[0], c.normal[1], c.normal[2], uv[2][0], uv[2][1])
                    elif len(verts) == 4:
                        a = mesh.verts[verts[0]]
                        b = mesh.verts[verts[1]]
                        c = mesh.verts[verts[2]]
                        
                        self.add_face(a.co[0]*scale, a.co[1]*scale, a.co[2]*scale, a.normal[0], a.normal[1], a.normal[2], uv[0][0], uv[0][1], b.co[0]*scale, b.co[1]*scale, b.co[2]*scale, b.normal[0], b.normal[1], b.normal[2], uv[1][0], uv[1][1], c.co[0]*scale, c.co[1]*scale, c.co[2]*scale, c.normal[0], c.normal[1], c.normal[2], uv[2][0], uv[2][1])
                        a = mesh.verts[verts[0]]
                        b = mesh.verts[verts[2]]
                        c = mesh.verts[verts[3]]
                        
                        self.add_face(a.co[0]*scale, a.co[1]*scale, a.co[2]*scale, a.normal[0], a.normal[1], a.normal[2], uv[0][0], uv[0][1], b.co[0]*scale, b.co[1]*scale, b.co[2]*scale, b.normal[0], b.normal[1], b.normal[2], uv[2][0], uv[2][1], c.co[0]*scale, c.co[1]*scale, c.co[2]*scale, c.normal[0], c.normal[1], c.normal[2], uv[3][0], uv[3][1])
                    else:
                        print("cannot handle so many faces")
        
                # my mind is not working today
                cnt = 0
                framecount = 0
                for fn in range(frame_start, frame_end + 1):
                    cnt = cnt + 1
                    if cnt == 2:
                        cnt = 0
                        continue
                    framecount = framecount + 1

                file = open(filename, 'wb')
                file.write(struct.pack('<4s', 'ALM1'))
                file.write(struct.pack('<i', 0)) # flags
                file.write(struct.pack('<i', len(self.verts))) # vertex count
                file.write(struct.pack('<i', len(self.faces))) # face count
                file.write(struct.pack('<i', framecount)) # frame count
                cnt = 0
                for fn in range(frame_start, frame_end + 1):
                    cnt = cnt + 1
                    if cnt == 2:
                        cnt = 0
                        continue
                    print("trying to export frame %d" % (fn))
                    self.verts = []
                    self.faces = []
                    scene.set_frame(fn)
                    if EXPORT_APPLY_MODIFIERS:
                            mesh = ob.create_mesh(scene, True, 'PREVIEW')
                    else:
                            mesh = ob.data.copy()
                    for i, f in enumerate(mesh.faces):
                        verts = f.verts
                        uv = uv_layer[i]
                        uv = uv.uv1, uv.uv2, uv.uv3, uv.uv4    
                        if len(verts) == 3:
                            a = mesh.verts[verts[0]]
                            b = mesh.verts[verts[1]]
                            c = mesh.verts[verts[2]]
                            
                            self.add_face(a.co[0]*scale, a.co[1]*scale, a.co[2]*scale, a.normal[0], a.normal[1], a.normal[2], uv[0][0], uv[0][1], b.co[0]*scale, b.co[1]*scale, b.co[2]*scale, b.normal[0], b.normal[1], b.normal[2], uv[1][0], uv[1][1], c.co[0]*scale, c.co[1]*scale, c.co[2]*scale, c.normal[0], c.normal[1], c.normal[2], uv[2][0], uv[2][1])
                        elif len(verts) == 4:
                            a = mesh.verts[verts[0]]
                            b = mesh.verts[verts[1]]
                            c = mesh.verts[verts[2]]
                            
                            self.add_face(a.co[0]*scale, a.co[1]*scale, a.co[2]*scale, a.normal[0], a.normal[1], a.normal[2], uv[0][0], uv[0][1], b.co[0]*scale, b.co[1]*scale, b.co[2]*scale, b.normal[0], b.normal[1], b.normal[2], uv[1][0], uv[1][1], c.co[0]*scale, c.co[1]*scale, c.co[2]*scale, c.normal[0], c.normal[1], c.normal[2], uv[2][0], uv[2][1])
                            a = mesh.verts[verts[0]]
                            b = mesh.verts[verts[2]]
                            c = mesh.verts[verts[3]]
                            
                            self.add_face(a.co[0]*scale, a.co[1]*scale, a.co[2]*scale, a.normal[0], a.normal[1], a.normal[2], uv[0][0], uv[0][1], b.co[0]*scale, b.co[1]*scale, b.co[2]*scale, b.normal[0], b.normal[1], b.normal[2], uv[2][0], uv[2][1], c.co[0]*scale, c.co[1]*scale, c.co[2]*scale, c.normal[0], c.normal[1], c.normal[2], uv[3][0], uv[3][1])
                    for v in self.verts:
                        file.write(struct.pack('<ffffffff', v[0], v[2], -v[1], v[3], v[5], -v[4], v[6], 1.0-v[7]))

                for f in self.faces:
                    file.write(struct.pack('<HHH', f[0], f[1], f[2]))
                
                file.close()

from bpy.props import *
class EXPORT_OT_alm(bpy.types.Operator):
        '''Export a single object as a ALM (Alithia Model) file.'''
        bl_idname = "export.alm"
        bl_label = "Export Alithia Model (ALM)"

        path = StringProperty(name="File Path", description="File path used for exporting the ALM file", maxlen=1024)
        use_modifiers = BoolProperty(name="Apply Modifiers", description="Apply modifiers to the exported mesh", default=True)
        scale = FloatProperty(name="Scale", description="Vertex scale", min=0.01, max=1000.0, soft_min=0.01, soft_max=1000.0, default=1.0)
        frame_start = IntProperty(name="Start Frame", description="Start frame", min=0, max=9999999, default=1)
        frame_end = IntProperty(name="End Frame", description="End frame", min=0, max=9999999, default=1)

        def poll(self, context):
                return context.active_object != None

        def execute(self, context):
                if not self.properties.path:
                        raise Exception("Filename not set")

                exporter = AlithiaModelExporter()
                exporter.write(self.properties.path, context.scene, context.active_object, self.properties.scale, self.properties.frame_start, self.properties.frame_end, EXPORT_APPLY_MODIFIERS = self.properties.use_modifiers)

                return {'FINISHED'}

        def invoke(self, context, event):
                wm = context.manager
                wm.add_fileselect(self)
                return {'RUNNING_MODAL'}

def menu_func(self, context):
    import os
    default_path = os.path.splitext(bpy.data.filepath)[0] + ".alm"
    self.layout.operator(EXPORT_OT_alm.bl_idname, text="Alithia Model (.alm)").path = default_path


def register():
    bpy.types.register(EXPORT_OT_alm)
    bpy.types.INFO_MT_file_export.append(menu_func)


def unregister():
    bpy.types.unregister(EXPORT_OT_alm)
    bpy.types.INFO_MT_file_export.remove(menu_func)

if __name__ == "__main__":
    register()
