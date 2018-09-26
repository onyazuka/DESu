# DESu DES
DES encryption + Triple DES + Multithread version

<h2>Overview</h2>
C++ implementation of DES - Data Encryption Standart algorithm.

<h2>Usage</h2>

Usage: DES mode [settings] keys_file input_file output_file<br/>
Modes: -g - key file generation, -e - encrypt, -d - decrypt.<br/>
settings: -3 eee3 || ede3 - triple DES</br>
          -mt - multithread mode<br/>
Key file generation: DES -g keys_number keyfile_name<br/>
