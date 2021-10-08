import cv2
import numpy as np
import time

def numpy_color2gray(image_path):
    t0 = time.time()

    image = image_path
    if (isinstance(image_path, str)):
        image = cv2.imread(image_path)
        if image is None:
            print("Failed to load image.")
            exit()
        image = np.array(image)

    """
    Grayscaling the image by weighting the three channels, and then taking the average
    of the three to get a grayscale.
    """
    R, G, B = image[:,:,2], image[:,:,1], image[:,:,0]
    R = R*0.21
    G = G*0.72
    B = B*0.07

    plus = R+B+G
    grayscale_image = image

    for i in range(3):
        grayscale_image[:,:,i] = plus

    grayscale_image = grayscale_image.astype("uint8")
    return grayscale_image

