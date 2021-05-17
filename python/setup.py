from setuptools import setup, find_packages

setup(
    name='sdmt',
    packages=find_packages(),
    entry_points={
        "console_scripts": [
            'sdmt=sdmt.scripts:main'
        ]
    }
)
