import cv2
import numpy as np

f = open("capture96x96.raw", "rb")
data = np.fromfile(f, dtype=np.uint16) 
f.close()

r_raw = np.right_shift(np.bitwise_and(data, 0b0000000011111000), 3, dtype=np.uint16)
b_raw = np.right_shift(np.bitwise_and(data, 0b0001111100000000), 8, dtype=np.uint16)
gh = np.left_shift(np.bitwise_and(data, 0b0000000000000111), 3, dtype=np.uint16)
gl = np.right_shift(np.bitwise_and(data, 0b1110000000000000), 13, dtype=np.uint16)
g_raw = np.bitwise_or(gh, gl)

# normalize to 1 for opencv
r_scalar = 1 / 32
g_scalar = 1 / 64
b_scalar = 1 / 32

r_scaled = np.multiply(r_raw, r_scalar).reshape((96, 96))
g_scaled = np.multiply(g_raw, g_scalar).reshape((96, 96))
b_scaled = np.multiply(b_raw, b_scalar).reshape((96, 96))

# opencv defaults BGR instead of RGB
image = cv2.merge([b_scaled, g_scaled, r_scaled])

#cv2.imshow("test", image)
cv2.imshow("test", cv2.resize(image, dsize=(500, 500), interpolation=cv2.INTER_CUBIC))
cv2.waitKey()
cv2.destroyAllWindows()