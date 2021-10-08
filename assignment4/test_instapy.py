from instapy import *
import numpy as np
import cv2

def test_gray():
    array = make_array()
    clone1 = array*1
    clone2 = array*1
    clone3 = array*1
    python = python_color2gray(clone1)
    numpy = numpy_color2gray(clone2)
    numba = numba_color2gray(clone3)
    
    R, G, B = array[:,:,2], array[:,:,1], array[:,:,0]
    R = R*0.21
    G = G*0.72
    B = B*0.07

    plus = R+B+G
    grayscale_image = array

    for i in range(3):
        grayscale_image[:,:,i] = plus

    assert(python.all() == grayscale_image.all())
    assert(numpy.all() == grayscale_image.all())
    assert(numba.all() == grayscale_image.all())
    

def test_sepia():
    array = make_array()
    clone1 = array*1
    clone2 = array*1
    clone3 = array*1
    python = python_color2sepia(clone1)
    numpy = numpy_color2sepia(clone2)
    numba = numba_color2sepia(clone3)

    r, g, b = array[:,:,2], array[:,:,1], array[:,:,0]

    B = r*0.272 + g*0.534 + b*0.131
    G = r*0.349 + g*0.686 + b*0.168
    R = r*0.393 + g*0.769 + b*0.189

    B[B>255] = 255
    G[G>255] = 255
    R[R>255] = 255

    sepia_image = array

    sepia_image[:,:,0] = B
    sepia_image[:,:,1] = G
    sepia_image[:,:,2] = R

    i = np.random.randint(0, 500)
    j = np.random.randint(0, 600)

    assert(python[i,j].all() == array[i,j].all())
    assert(numpy[i,j].all()  == array[i,j].all())
    assert(numba[i,j].all()  == array[i,j].all())
    
    
def make_array():
    return np.random.randint(0, 255, size=(500, 600, 3))