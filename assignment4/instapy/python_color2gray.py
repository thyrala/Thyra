import cv2
import time
import numpy as np

def python_color2gray(image_path):
    t0 = time.time()

    image = image_path
    if (isinstance(image_path, str)):
        image = cv2.imread(image_path)
        if image is None:
            print("Failed to load image.")
            exit()
    grayscale_image = image

    """
    Iterates through all the pixels in the image, and applys weights for the
    red, green and blue channel.
    """
    for i in range(0, grayscale_image.shape[0]):
        for o in range(0, grayscale_image.shape[1]):
            grayscale_image[i, o] = (0.21*image[i, o, 2] + 0.72*image[i, o, 1] + 0.07*image[i, o, 0])

    grayscale_image = grayscale_image.astype("uint8")
    print(time.time() - t0)
    return grayscale_image
