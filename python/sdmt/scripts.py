"""
Copyright 2021 PIDL(Petabyte-scale In-memory Database Lab) http://kdb.snu.ac.kr
This work was supported by Next-Generation Information Computing Development
Program through the National Research Foundation of Korea(NRF)
funded by the Ministry of Science, ICT (NRF-2016M3C4A7952587)
Author: Ilju Lee, Jongin Kim, Hyerim Jeon, Youngjune Park
Contact: sdmt@kdb.snu.ac.kr
"""

import click
from clint.textui import puts, colored

@click.group()
def sdmt_cli():
    pass

@click.command()
@click.option(
    '--filename',
    required=True,
    help='csv file path')
def analysis(filename):
    print('get statistics of graph data')

sdmt_cli.add_command(analysis)

def main():
    sdmt_cli()

if __name__=='__main__':
    main() 
