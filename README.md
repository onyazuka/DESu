# DESu DES
DES encryption + Triple DES + Multithread version

<h2>Overview</h2>
C++ implementation of DES - Data Encryption Standart algorithm.<br/>
Supports Triple DES, also can use some of your cores.<br/>
Tested and optimized for msvc 2017 and gcc.<br/>

<h2>MSVC compilation</h2>
<ol>
  <li>Download repository</li>
  <li>Run DES.sln</li>
  <li>Press Ctrl+F5</li>
  <li>Enjoy</li>
</ol>

<h2>Usage</h2>

<b>Usage</b>: DES mode [settings] keys_file input_file output_file<br/>
<b>Key file generation for DES</b>: DES -g 1 keyfile_name<br/>
<b>Key file generation for Triple-DES</b>: DES -g 3 keyfile_name<br/>
<h3>Modes</h3>
<table>
  <tr>
    <td>-g</td>
    <td>key file generation</td>
  </tr>
  <tr>
    <td>-e</td>
    <td>encryptiont mode</td>
  </tr>
  <tr>
    <td>-d</td>
    <td>decryption mode</td>
  </tr>
</table>
<h3>Settings</h3>
<table>
  <tr>
    <td>-3 eee3</td>
    <td>Triple DES (DES-EEE3)</td>
  </tr>
  <tr>
    <td>-3 ede3</td>
    <td>Triple DES (DES-EDE3)</td>
  </tr>
  <tr>
    <td>-mt</td>
    <td>Multithread mode</td>
  </tr>
</table>

<h2>Examples</h2>

<h3>Example: using command-line utility for encrypting</h3>

<p>Generating key file</p>
<pre>DES -g 1 keys.key</pre>
<p>Encrypting file</p>
<pre>DES -e keys.key input.bin input.enc</pre>
<p>Decrypting file</p>
<pre>DES -d keys.key input.enc input_decrypted.bin</pre>
<p>Encrypting Triple DES EEE3</p>
<pre>DES -e -3 eee3 keys.key input.bin input.enc</pre>
<p>Using multithreading</p>
<pre>DES -e -3 eee3 -mt keys.key input.bin input.enc</pre>


<h3>Example: encrypt raw block of data</h3>

<p>Here we are just using DESEncrypter class from DES.h</p>
<pre>
#include "DES.h"
uint64_t data = //...your data;
uint64_t key = //...your key;
DESEncrypter encr{ data, key, DESEncrypter::Mode::ENCRYPT };
uint64_t output = encr.run(); //your encrypted data
</pre>

<h3>Example: decrypt raw block of data</h3>

<pre>
#include "DES.h"
uint64_t data = //...your data;
uint64_t key = //...your key;
DESEncrypter encr{ data, key, DESEncrypter::Mode::DECRYPT };
uint64_t output = encr.run(); //your encrypted data
</pre>

<h3>Example: encrypring file</h3>
<p>Here you should initialize instance of File_Crypter from DESFileCrypt.h</p>
<pre>
#include "DESFileCrypt.h"
fc = File_Crypter{};
fc.ifname = "/foo/bar"    // file you want to encrypt
fc.kname = "/foo/keys"    // file 
fc.ofname = "/foo/output" // output file
fc.read_keys();           // reads keys from keyfile and populates fc object with it
fc.mode = fc.Encrypt;     // fc.Encrypt or fc.Decrypt
fc.triple_des = true;     // if you want Triple_DES
fc.set_triple_des_mode(fc.EEE3);  // if you want Triple_DES
fc.multithread = true;    // if you want multithread
fc.run();
</pre>
