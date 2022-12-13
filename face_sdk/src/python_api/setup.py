from setuptools import setup, find_packages

__version__ = '0.0.1'


if __name__ == '__main__':
    setup(
        name="face_sdk",
        version=__version__,
        description="Package allows you to use FaceSDK in python language",
        packages=find_packages(),
        python_requires='>=3.6',
        install_requires=["numpy>=1.16.2,<1.25", "opencv-python<=4.6.0.66,>4", "multipledispatch"],
    )
