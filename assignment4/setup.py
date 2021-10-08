from setuptools import setup, find_packages

setup(name = 'Instapy',
        version = '1.0', 
        description = 'Instgram filters',
        author = 'Thyra L. Aakvag',
        packages = find_packages(), 
        install_requires=['numba', 'numpy', 'opencv-python'], 
        scripts = ['bin/instapy']
        )