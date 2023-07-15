import cv2
import numpy as np

f = open("console_capture_32x32.log")
array = []
for line in f:
    for x in line.split():
        array.append(int(x, 16))
f.close()

byte_array = bytearray(array)

raw = np.array(byte_array, dtype=np.uint8)
raw = raw.reshape((1024, 3))

r = raw[:,0].reshape((32, 32))
g = raw[:,1].reshape((32, 32))
b = raw[:,2].reshape((32, 32))

r_max = np.max(r)
g_max = np.max(g)
b_max = np.max(b)

b_int = np.array(b / b_max * 255, dtype=np.uint8)
g_int = np.array(g / g_max * 255, dtype=np.uint8)
r_int = np.array(r / r_max * 255, dtype=np.uint8)

image = cv2.merge([b_int, g_int, r_int])
image_rgb = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
cv2.imwrite("console_capture.png", image_rgb)

image_bgr = cv2.merge([b/b_max, g/g_max, r/r_max])
cv2.imshow("", image_bgr)
cv2.waitKey()
cv2.destroyAllWindows()