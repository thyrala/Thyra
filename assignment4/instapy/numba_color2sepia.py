import time
from numba import jit
import cv2

def numba_color2sepia(image_path):

    image = image_path
    if (isinstance(image_path, str)):
        image = cv2.imread(image_path)
        if image is None:
            print("Failed to load image.")
            exit()
    
    numb(image)
    t0 = time.time()

    sepia_image = numb(image)

    sepia_image = sepia_image.astype("uint8")
    return sepia_image


@jit
def numb(image):
    sepia_image = image
    """
    Iterates through all the pixels in the image, and applys weights for the
    red, green and blue channel.
    """
    for i in range(0, image.shape[0]):
        for o in range(0, image.shape[1]):
            (b, g, r) = (image[i][o][0], image[i][o][1], image[i][o][2])

            R = int(r*0.393 + g*0.769 + b*0.189)
            G = int(r*0.349 + g*0.686 + b*0.168)
            B = int(r*0.272 + g*0.534 + b*0.131)
        
            if R > 255: R = 255
            if G > 255: G = 255
            if B > 255: B = 255

            sepia_image[i][o][0] = B
            sepia_image[i][o][1] = G
            sepia_image[i][o][2] = R
    return sepia_image