"""
Copyright 2021 PIDL(Petabyte-scale In-memory Database Lab) http://kdb.snu.ac.kr
This work was supported by Next-Generation Information Computing Development
Program through the National Research Foundation of Korea(NRF)
funded by the Ministry of Science, ICT (NRF-2016M3C4A7952587)
Author: Ilju Lee, Jongin Kim, Hyerim Jeon, Youngjune Park
Contact: sdmt@kdb.snu.ac.kr
""" 

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
