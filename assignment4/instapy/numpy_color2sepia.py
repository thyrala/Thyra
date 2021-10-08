import cv2
import time
import numpy as np

def numpy_color2sepia(image_path):
    t0 = time.time()

    image = image_path
    if (isinstance(image_path, str)):
        image = cv2.imread(image_path)
        if image is None:
            print("Failed to load image.")
            exit()

    sepia_image = image

    """
    Iterates through all the pixels in the image, and applys weights for the
    red, green and blue channel.
    """


    r, g, b = image[:,:,2], image[:,:,1], image[:,:,0]

    B = r*0.272 + g*0.534 + b*0.131
    G = r*0.349 + g*0.686 + b*0.168
    R = r*0.393 + g*0.769 + b*0.189

    B[B>255] = 255
    G[G>255] = 255
    R[R>255] = 255

    sepia_image[:,:,0] = B
    sepia_image[:,:,1] = G
    sepia_image[:,:,2] = R

    sepia_image = sepia_image.astype("uint8")
    return sepia_image