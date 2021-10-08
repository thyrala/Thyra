import time
from numba import jit
import cv2

def numba_color2gray(image_path):

    image = image_path
    if (isinstance(image_path, str)):
        image = cv2.imread(image_path)

        if image is None:
            print("Failed to load image.")
            exit()
        image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)

    numb(image)
    t0 = time.time()

    grayscale_image = numb(image)
 
    grayscale_image = grayscale_image.astype("uint8")
    return grayscale_image

@jit
def numb(image):
    """
    Iterates through all the pixels in the image, and applys weights for the
    red, green and blue channel.
    """
    grayscale_image = image
    for i in range(0, grayscale_image.shape[0]):
        for o in range(0, grayscale_image.shape[1]):
            grayscale_image[i, o] = (0.21*image[i, o, 2] + 0.72*image[i, o, 1] + 0.07*image[i, o, 0])
    return grayscale_image;
