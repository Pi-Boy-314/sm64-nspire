import sys
def ppm(p):
    b = open(p, 'rb').read(); parts = b.split(maxsplit=4)
    w, h = int(parts[1]), int(parts[2]); return w, h, bytearray(parts[4][:w*h*3])
a_p, b_p, out, scale = sys.argv[1], sys.argv[2], sys.argv[3], int(sys.argv[4])
w, h, A = ppm(a_p); _, _, B = ppm(b_p)
D = bytearray(w*h*3); big = small = 0
for i in range(w*h):
    d = max(abs(A[3*i+k]-B[3*i+k]) for k in range(3))
    g = (A[3*i]+A[3*i+1]+A[3*i+2])//9
    if d > 16: D[3*i:3*i+3] = b'\xff\x00\x00'; big += 1
    elif d: D[3*i:3*i+3] = b'\xff\xff\x00'; small += 1
    else: D[3*i:3*i+3] = bytes((g, g, g))
print(f"{a_p.split('/')[-3:]}: >16: {big} px, 1-16: {small} px of {w*h}")
W = w*3*scale; H = h*scale; o = bytearray(W*H*3)
for y in range(H):
    for x in range(W):
        img = (A, B, D)[x // (w*scale)]; sx = (x % (w*scale)) // scale; sy = y // scale
        o[3*(y*W+x):3*(y*W+x)+3] = img[3*(sy*w+sx):3*(sy*w+sx)+3]
open(out, 'wb').write(b'P6 %d %d 255\n' % (W, H) + o)
