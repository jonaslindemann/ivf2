"""Generate examples/assets/models/showcase.glb, the test model of the gltf1 example.

Everything is made here (geometry, PNG textures, animations), so the model needs no
third-party asset. It uses every glTF feature ivf supports:

- a node hierarchy (a mast with a spinning rotor),
- PBR materials: base colour, metallic-roughness, normal, occlusion and emissive textures,
  alpha blend (a glass pane) and alpha mask (a cut-out leaf), vertex colours,
- a skinned tentacle (six joints) and a blob with two morph targets,
- two animations: "Showcase" (linear, step and cubic spline channels) and "Idle".

Usage (from the repository root):  python examples/gltf1/make_showcase.py
"""

import json
import math
import pathlib
import struct
import zlib

OUT = pathlib.Path(__file__).resolve().parents[1] / 'assets' / 'models' / 'showcase.glb'


# --------------------------------------------------------------------------- PNG

def png(width, height, pixel):
    """PNG bytes; pixel(x, y) returns an (r, g, b, a) tuple of 0-255 ints. Row 0 is the top."""
    raw = bytearray()
    for y in range(height):
        raw.append(0)
        for x in range(width):
            raw.extend(pixel(x, y))

    def chunk(tag, data):
        return struct.pack('>I', len(data)) + tag + data + struct.pack('>I', zlib.crc32(tag + data) & 0xffffffff)

    return (b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', width, height, 8, 6, 0, 0, 0)) +
            chunk(b'IDAT', zlib.compress(bytes(raw), 9)) + chunk(b'IEND', b''))


def clamp8(v):
    return max(0, min(255, int(round(v))))


def tex_painted(x, y):
    # Blue panels with lighter seams and a few rivets
    u, v = x / 128.0, y / 128.0
    seam = min(abs(math.sin(u * math.pi * 4)), abs(math.sin(v * math.pi * 4)))
    base = (60, 110, 200) if (int(u * 4) + int(v * 4)) % 2 == 0 else (70, 130, 215)
    k = 0.75 + 0.25 * min(seam * 8, 1)
    return (clamp8(base[0] * k), clamp8(base[1] * k), clamp8(base[2] * k), 255)


def tex_metal_rough(x, y):
    # glTF: roughness in G, metallic in B (occlusion in R, used as AO map too)
    u, v = x / 128.0, y / 128.0
    seam = min(abs(math.sin(u * math.pi * 4)), abs(math.sin(v * math.pi * 4)))
    rough = 0.25 + 0.5 * ((int(u * 8) + int(v * 2)) % 2)
    ao = 0.55 + 0.45 * min(seam * 6, 1)
    return (clamp8(ao * 255), clamp8(rough * 255), 255, 255)


def tex_normal(x, y):
    # Bumps: tangent-space normals of a cosine height field
    u, v = x / 128.0 * math.pi * 8, y / 128.0 * math.pi * 8
    dx, dy = -0.5 * math.sin(u) * math.cos(v), -0.5 * math.cos(u) * math.sin(v)
    n = (dx, dy, 1.0)
    length = math.sqrt(sum(c * c for c in n))
    return tuple(clamp8((c / length * 0.5 + 0.5) * 255) for c in n) + (255,)


def tex_emissive(x, y):
    # Glowing grid on a dark panel
    on = (x % 32) < 3 or (y % 32) < 3
    return (255, 170, 60, 255) if on else (10, 10, 20, 255)


def tex_leaf(x, y):
    # A leaf with holes: alpha 0 outside the leaf and in the holes
    u, v = (x + 0.5) / 128.0 * 2 - 1, (y + 0.5) / 128.0 * 2 - 1
    inside = (u * u) / 0.45 + (v * v) / 0.9 < 1
    hole = ((u * 6) % 1 - 0.5) ** 2 + ((v * 6) % 1 - 0.5) ** 2 < 0.06
    vein = abs(u) < 0.03
    g = 140 + int(60 * v)
    return (40, g, 50, 255) if (inside and (vein or not hole)) else (0, 0, 0, 0)


# --------------------------------------------------------------------------- geometry

def box(sx, sy, sz):
    """Positions, normals, uvs, indices of a box with half extents."""
    p, n, t, idx = [], [], [], []
    faces = [((1, 0, 0), (0, 0, -1), (0, 1, 0)), ((-1, 0, 0), (0, 0, 1), (0, 1, 0)),
             ((0, 1, 0), (1, 0, 0), (0, 0, -1)), ((0, -1, 0), (1, 0, 0), (0, 0, 1)),
             ((0, 0, 1), (1, 0, 0), (0, 1, 0)), ((0, 0, -1), (-1, 0, 0), (0, 1, 0))]
    for normal, uaxis, vaxis in faces:
        base = len(p)
        for su, sv in ((-1, -1), (1, -1), (1, 1), (-1, 1)):
            q = [normal[i] + su * uaxis[i] + sv * vaxis[i] for i in range(3)]
            p.append((q[0] * sx, q[1] * sy, q[2] * sz))
            n.append(normal)
            t.append(((su + 1) / 2, (1 - sv) / 2))
        idx += [base, base + 1, base + 2, base, base + 2, base + 3]
    return p, n, t, idx


def cylinder(radius, height, slices=32, rings=1, y0=0.0):
    p, n, t, idx = [], [], [], []
    for r in range(rings + 1):
        y = y0 + height * r / rings
        for s in range(slices + 1):
            a = 2 * math.pi * s / slices
            p.append((radius * math.cos(a), y, radius * math.sin(a)))
            n.append((math.cos(a), 0, math.sin(a)))
            t.append((s / slices, 1 - r / rings))
    for r in range(rings):
        for s in range(slices):
            a, b = r * (slices + 1) + s, (r + 1) * (slices + 1) + s
            idx += [a, b, a + 1, a + 1, b, b + 1]
    # caps
    for y, ny in ((y0, -1), (y0 + height, 1)):
        center = len(p)
        p.append((0, y, 0))
        n.append((0, ny, 0))
        t.append((0.5, 0.5))
        for s in range(slices + 1):
            a = 2 * math.pi * s / slices
            p.append((radius * math.cos(a), y, radius * math.sin(a)))
            n.append((0, ny, 0))
            t.append((0.5 + 0.5 * math.cos(a), 0.5 + 0.5 * math.sin(a)))
        for s in range(slices):
            if ny > 0:
                idx += [center, center + 2 + s, center + 1 + s]
            else:
                idx += [center, center + 1 + s, center + 2 + s]
    return p, n, t, idx


def sphere(radius, slices=48, stacks=24):
    p, n, t, idx = [], [], [], []
    for i in range(stacks + 1):
        th = math.pi * i / stacks
        for j in range(slices + 1):
            ph = 2 * math.pi * j / slices
            d = (math.sin(th) * math.cos(ph), math.cos(th), math.sin(th) * math.sin(ph))
            p.append(tuple(radius * c for c in d))
            n.append(d)
            t.append((j / slices, i / stacks))
    for i in range(stacks):
        for j in range(slices):
            a, b = i * (slices + 1) + j, (i + 1) * (slices + 1) + j
            idx += [a, a + 1, b, a + 1, b + 1, b]
    return p, n, t, idx


def quad(w, h):
    p = [(-w, -h, 0), (w, -h, 0), (w, h, 0), (-w, h, 0)]
    n = [(0, 0, 1)] * 4
    t = [(0, 1), (1, 1), (1, 0), (0, 0)]
    return p, n, t, [0, 1, 2, 0, 2, 3]


# --------------------------------------------------------------------------- GLB writer

class Glb:
    def __init__(self):
        self.bin = bytearray()
        self.json = {'asset': {'version': '2.0', 'generator': 'ivf2 make_showcase.py'},
                     'buffers': [], 'bufferViews': [], 'accessors': [], 'images': [], 'textures': [],
                     'samplers': [{'magFilter': 9729, 'minFilter': 9987, 'wrapS': 10497, 'wrapT': 10497}],
                     'materials': [], 'meshes': [], 'nodes': [], 'skins': [], 'animations': [],
                     'scenes': [{'nodes': []}], 'scene': 0}

    def view(self, data, target=None):
        while len(self.bin) % 4:
            self.bin.append(0)
        offset = len(self.bin)
        self.bin.extend(data)
        v = {'buffer': 0, 'byteOffset': offset, 'byteLength': len(data)}
        if target:
            v['target'] = target
        self.json['bufferViews'].append(v)
        return len(self.json['bufferViews']) - 1

    def accessor(self, values, type_, component=5126, target=None, minmax=False):
        width = {'SCALAR': 1, 'VEC2': 2, 'VEC3': 3, 'VEC4': 4, 'MAT4': 16}[type_]
        flat = [c for v in values for c in (v if isinstance(v, (tuple, list)) else (v,))]
        fmt = {5126: 'f', 5123: 'H', 5125: 'I', 5121: 'B'}[component]
        a = {'bufferView': self.view(struct.pack('<%d%s' % (len(flat), fmt), *flat), target),
             'componentType': component, 'count': len(values), 'type': type_}
        if minmax:
            a['min'] = [min(flat[i::width]) for i in range(width)]
            a['max'] = [max(flat[i::width]) for i in range(width)]
        self.json['accessors'].append(a)
        return len(self.json['accessors']) - 1

    def texture(self, name, data):
        self.json['images'].append({'name': name, 'mimeType': 'image/png', 'bufferView': self.view(data)})
        self.json['textures'].append({'source': len(self.json['images']) - 1, 'sampler': 0})
        return len(self.json['textures']) - 1

    def material(self, m):
        self.json['materials'].append(m)
        return len(self.json['materials']) - 1

    def mesh(self, name, geometry, material, colors=None, joints=None, weights=None, targets=None,
             target_weights=None):
        p, n, t, idx = geometry
        attrs = {'POSITION': self.accessor(p, 'VEC3', target=34962, minmax=True),
                 'NORMAL': self.accessor(n, 'VEC3', target=34962),
                 'TEXCOORD_0': self.accessor(t, 'VEC2', target=34962)}
        if colors:
            attrs['COLOR_0'] = self.accessor(colors, 'VEC4', target=34962)
        if joints:
            attrs['JOINTS_0'] = self.accessor(joints, 'VEC4', component=5123, target=34962)
            attrs['WEIGHTS_0'] = self.accessor(weights, 'VEC4', target=34962)
        prim = {'attributes': attrs, 'indices': self.accessor(idx, 'SCALAR', component=5125, target=34963),
                'material': material}
        if targets:
            prim['targets'] = [{'POSITION': self.accessor(d, 'VEC3', minmax=True)} for d in targets]
        m = {'name': name, 'primitives': [prim]}
        if target_weights:
            m['weights'] = target_weights
        self.json['meshes'].append(m)
        return len(self.json['meshes']) - 1

    def node(self, node, root=False):
        self.json['nodes'].append(node)
        index = len(self.json['nodes']) - 1
        if root:
            self.json['scenes'][0]['nodes'].append(index)
        return index

    def write(self, path):
        while len(self.bin) % 4:
            self.bin.append(0)
        self.json['buffers'].append({'byteLength': len(self.bin)})
        text = json.dumps(self.json, separators=(',', ':')).encode()
        text += b' ' * (-len(text) % 4)
        body = (struct.pack('<II', len(text), 0x4E4F534A) + text +
                struct.pack('<II', len(self.bin), 0x004E4942) + bytes(self.bin))
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(struct.pack('<III', 0x46546C67, 2, 12 + len(body)) + body)


def quat(axis, degrees):
    a = math.radians(degrees) / 2
    s = math.sin(a)
    return [axis[0] * s, axis[1] * s, axis[2] * s, math.cos(a)]


def main():
    g = Glb()

    # Textures and materials
    painted = g.texture('painted', png(128, 128, tex_painted))
    metal_rough = g.texture('metal_rough_ao', png(128, 128, tex_metal_rough))
    normal = g.texture('bumps', png(128, 128, tex_normal))
    emissive = g.texture('grid', png(128, 128, tex_emissive))
    leaf = g.texture('leaf', png(128, 128, tex_leaf))

    m_painted = g.material({'name': 'Painted metal',
                            'pbrMetallicRoughness': {'baseColorTexture': {'index': painted},
                                                     'metallicRoughnessTexture': {'index': metal_rough},
                                                     'metallicFactor': 1.0, 'roughnessFactor': 1.0},
                            'occlusionTexture': {'index': metal_rough},
                            'normalTexture': {'index': normal}})
    m_rubber = g.material({'name': 'Rubber', 'pbrMetallicRoughness': {
        'baseColorFactor': [0.85, 0.2, 0.25, 1], 'metallicFactor': 0.0, 'roughnessFactor': 0.55}})
    m_gold = g.material({'name': 'Gold', 'pbrMetallicRoughness': {
        'baseColorFactor': [1.0, 0.78, 0.34, 1], 'metallicFactor': 1.0, 'roughnessFactor': 0.25}})
    m_panel = g.material({'name': 'Glowing panel', 'pbrMetallicRoughness': {
        'baseColorFactor': [0.05, 0.05, 0.06, 1], 'metallicFactor': 0.0, 'roughnessFactor': 0.6},
        'emissiveTexture': {'index': emissive}, 'emissiveFactor': [1, 1, 1],
        'extensions': {'KHR_materials_emissive_strength': {'emissiveStrength': 3.0}}})
    m_glass = g.material({'name': 'Glass', 'alphaMode': 'BLEND', 'pbrMetallicRoughness': {
        'baseColorFactor': [0.6, 0.85, 1.0, 0.3], 'metallicFactor': 0.0, 'roughnessFactor': 0.05}})
    m_leaf = g.material({'name': 'Leaf', 'alphaMode': 'MASK', 'alphaCutoff': 0.5, 'doubleSided': True,
                         'pbrMetallicRoughness': {'baseColorTexture': {'index': leaf}, 'metallicFactor': 0.0,
                                                  'roughnessFactor': 0.7}})
    m_vertex = g.material({'name': 'Vertex colours', 'pbrMetallicRoughness': {
        'metallicFactor': 0.0, 'roughnessFactor': 0.45}})
    g.json['extensionsUsed'] = ['KHR_materials_emissive_strength']

    # Pedestal with vertex colours (a rainbow around it)
    ped = cylinder(3.0, 0.2, slices=64)
    colors = []
    for (x, y, z) in ped[0]:
        a = math.atan2(z, x)
        colors.append((0.55 + 0.45 * math.cos(a), 0.55 + 0.45 * math.cos(a + 2.09), 0.55 + 0.45 * math.cos(a + 4.19), 1.0))
    g.node({'name': 'Pedestal', 'mesh': g.mesh('Pedestal', ped, m_vertex, colors=colors)}, root=True)

    # Mast with a spinning rotor (hierarchy)
    blade = g.mesh('Blade', box(0.7, 0.03, 0.12), m_painted)
    blades = [g.node({'name': 'Blade %d' % i, 'mesh': blade, 'translation': [0.75 * math.cos(math.radians(120 * i)), 0,
                                                                             -0.75 * math.sin(math.radians(120 * i))],
                      'rotation': quat((0, 1, 0), 120 * i)}) for i in range(3)]
    rotor = g.node({'name': 'Rotor', 'translation': [0, 2.3, 0], 'children': blades,
                    'mesh': g.mesh('Hub', sphere(0.15, 24, 12), m_gold)})
    mast = g.node({'name': 'Mast', 'translation': [0, 0.2, -1.5], 'children': [rotor],
                   'mesh': g.mesh('Mast', cylinder(0.07, 2.2, 16), m_painted)}, root=True)

    # Glowing panel behind, glass pane and a cut-out leaf in front
    g.node({'name': 'Panel', 'translation': [0, 1.3, -2.6], 'mesh': g.mesh('Panel', box(1.4, 0.8, 0.05), m_panel)},
           root=True)
    g.node({'name': 'Glass', 'translation': [1.7, 1.0, 1.3], 'rotation': quat((0, 1, 0), -30),
            'mesh': g.mesh('Glass', box(0.6, 0.8, 0.02), m_glass)}, root=True)
    leaf_node = g.node({'name': 'Leaf', 'translation': [-1.6, 1.2, 1.5],
                        'mesh': g.mesh('Leaf', quad(0.45, 0.8), m_leaf)}, root=True)

    # Skinned tentacle: a tube along Y with six joints
    segments, joint_count, height = 48, 6, 2.6
    tube = cylinder(0.16, height, slices=20, rings=segments, y0=0.0)
    joints, weights = [], []
    for (x, y, z) in tube[0]:
        f = min(max(y / height * (joint_count - 1), 0), joint_count - 1 - 1e-6)
        j = int(f)
        w = f - j
        joints.append((j, min(j + 1, joint_count - 1), 0, 0))
        weights.append((1 - w, w, 0, 0))
    # Taper the tube towards the tip
    tube = ([(x * (1 - 0.7 * y / height), y, z * (1 - 0.7 * y / height)) for (x, y, z) in tube[0]],) + tube[1:]
    seg = height / (joint_count - 1)
    joint_nodes = []
    parent = None
    for j in range(joint_count):
        idx = g.node({'name': 'Joint %d' % j, 'translation': [0, 0 if j == 0 else seg, 0]})
        if parent is not None:
            g.json['nodes'][parent].setdefault('children', []).append(idx)
        joint_nodes.append(idx)
        parent = idx
    ibm = []
    for j in range(joint_count):
        ibm.append((1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, -seg * j, 0, 1))
    g.json['skins'].append({'name': 'Tentacle skin', 'joints': joint_nodes,
                            'inverseBindMatrices': g.accessor(ibm, 'MAT4')})
    armature = g.node({'name': 'Armature', 'translation': [-1.5, 0.2, -0.2], 'children': [joint_nodes[0]]}, root=True)
    tentacle = g.node({'name': 'Tentacle', 'mesh': g.mesh('Tentacle', tube, m_rubber, joints=joints, weights=weights),
                       'skin': 0}, root=True)

    # Morphing blob: a sphere with "spikes" and "squash" targets
    sph = sphere(0.65)
    spikes, squash = [], []
    for (x, y, z), (nx, ny, nz) in zip(sph[0], sph[1]):
        k = 0.45 * max(0.0, math.sin(nx * 9) * math.sin(ny * 9) * math.sin(nz * 9))
        spikes.append((nx * k, ny * k, nz * k))
        squash.append((x * 0.35, -y * 0.45, z * 0.35))
    blob = g.node({'name': 'Blob', 'translation': [1.5, 0.85, -0.2],
                   'mesh': g.mesh('Blob', sph, m_gold, targets=[spikes, squash], target_weights=[0.0, 0.0])},
                  root=True)

    # Animations
    def channel(anim, node, path, times, values, interpolation='LINEAR'):
        typ = {'translation': 'VEC3', 'rotation': 'VEC4', 'scale': 'VEC3', 'weights': 'SCALAR'}[path]
        anim['samplers'].append({'input': g.accessor(times, 'SCALAR', minmax=True),
                                 'output': g.accessor(values, typ), 'interpolation': interpolation})
        anim['channels'].append({'sampler': len(anim['samplers']) - 1, 'target': {'node': node, 'path': path}})

    duration, keys = 4.0, 33
    show = {'name': 'Showcase', 'samplers': [], 'channels': []}
    times = [duration * i / (keys - 1) for i in range(keys)]
    channel(show, rotor, 'rotation', times, [quat((0, 1, 0), -360.0 * i / (keys - 1)) for i in range(keys)])
    for j in range(1, joint_count):
        values = [quat((0, 0, 1), 22.0 * math.sin(2 * math.pi * (t / duration) * 2 - j * 0.8)) for t in times]
        values = [[q[0], q[1], q[2], q[3]] for q in values]
        swing = [quat((1, 0, 0), 12.0 * math.cos(2 * math.pi * t / duration - j * 0.6)) for t in times]
        # combine: z wave * x swing
        combined = []
        for a, b in zip(values, swing):
            ax, ay, az, aw = a
            bx, by, bz, bw = b
            combined.append([aw * bx + ax * bw + ay * bz - az * by, aw * by - ax * bz + ay * bw + az * bx,
                             aw * bz + ax * by - ay * bx + az * bw, aw * bw - ax * bx - ay * by - az * bz])
        channel(show, joint_nodes[j], 'rotation', times, combined)
    # Blob weights with a cubic spline (in-tangent, value, out-tangent per key, two targets)
    wtimes = [0.0, 1.0, 2.0, 3.0, 4.0]
    wvalues = [(0, 0), (1, 0), (0, 0), (0, 1), (0, 0)]
    cubic = []
    for value in wvalues:
        cubic += [0.0, 0.0] + list(value) + [0.0, 0.0]
    show['samplers'].append({'input': g.accessor(wtimes, 'SCALAR', minmax=True),
                             'output': g.accessor(cubic, 'SCALAR'), 'interpolation': 'CUBICSPLINE'})
    show['channels'].append({'sampler': len(show['samplers']) - 1, 'target': {'node': blob, 'path': 'weights'}})
    # The leaf ticks round like a clock hand (step interpolation)
    channel(show, leaf_node, 'rotation', [float(i) for i in range(5)],
            [quat((0, 0, 1), -45.0 * i) for i in range(5)], 'STEP')
    g.json['animations'].append(show)

    idle = {'name': 'Idle', 'samplers': [], 'channels': []}
    channel(idle, rotor, 'rotation', times, [quat((0, 1, 0), -90.0 * i / (keys - 1)) for i in range(keys)])
    g.json['animations'].append(idle)

    del armature, tentacle, mast
    g.write(OUT)
    print('wrote', OUT, OUT.stat().st_size, 'bytes')


if __name__ == '__main__':
    main()
