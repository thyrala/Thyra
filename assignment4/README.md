# Assignment 4_Thyra

## Instapy package
Package which contains the instagram filters; sepia and grayscale.

## Package
### Requirements 
- python3
- numba
- numpy
- cv2

### How to install package

Go to "assigment4" directory and run
```
pip install .
```

### How to run 
#### Command Line arguments

```bash
-h, --help helpful message showing flags and usage of instapy

-f FILE, --file FILE The filename of file to apply filter to

-se, -- sepia Select sepia filter

-g, -- gray Select gray filter
-sc SCALE , -- scale SCALE Scale factor to resize image

-i {python , numba , numpy}, -- implement { python , numba , numpy , cython } Choose the implementation

-o OUT, -- out OUT The output filename

```

Example:
```
instapy -f <filename> -se (or) -g -i <implementation> -o <output_filename>
```

### instapy
Script which contains the user interface for the Instapy package.

```python
def grayscale_image(input_filename, output_filename=None, implementation = None, scale = None):
    """
    Takes in filename and runs the right implementation of the grayscale filter, which depends on the user input. If the user did not specify which implementation to use, the default implementation is numba.

    The user can specify the filename of the grayscale image, if not it will be saved as a default name.

    The user can also choose whether he/she wants to scale the image or not. If this is not specified, the grayscale image will have the same dimensions as the original file.

    The function returns the grayscale image.
    """

def sepia_image(input_filename, output_filename=None, implementation = None, scale = None):
    """
    Takes in filename and runs the right implementation of the sepia filter, which depends on the user input. If the user did not specify which implementation to use, the default implementation is numba.

    The user can specify the filename of the sepia image, if not it will be saved as a default name.

    The user can also choose whether he/she wants to scale the image or not. If this is not specified, the sepia image will have the same dimensions as the original file.

    The function returns the sepia image.
    """
```

### setup.py
Installs the instapy script in the Instapy package.

### test_instapy.py
Tests the Instapy package functions

#### Comand Line arguments
Example:
```
pytest
```

## Functions
### python_color2gray.py
This code takes in a numpy array and converts it into a image. Then is applies a grayscale filter to the image. It returns a numpy array version of the grayscale image.

### numpy_color2gray.py
This code takes in a numpy array and converts it into a image, using the numpy package. Then is applies a grayscale filter to the image. It returns a numpy array version of the grayscale image.

### numba_color2gray.py
This code takes in a numpy array and converts it into a image, using the numba package. Then is applies a grayscale filter to the image. It returns a numpy array version of the grayscale image.

### python_color2sepia.py
This code takes in a numpy array and converts it into a image. Then is applies a sepia filter to the image. It returns a numpy array version of the sepia image.

### numpy_color2sepia.py
This code takes in a numpy array and converts it into a image, using the numpy package. Then is applies a sepia filter to the image. It returns a numpy array version of the sepia image.

### numba_color2sepia.py
This code takes in a numpy array and converts it into a image, using the numba package. Then is applies a sepia filter to the image. It returns a numpy array version of the sepia image.
