Param(
    [uint16]$version_num,
    [int]$bram_width = 256
)

New-Item -Force init.coe
echo "memory_initialization_radix = 16 ;" | Out-File -Append init.coe
echo "memory_initialization_vector =" | Out-File -Append init.coe

for ($i=0; $i -lt ($bram_width - 1); $i++){
  echo "0000," | Out-File -Append init.coe
}
 $version_str = "{0:X4}" -f $version_num
 echo ($version_str + ";")| Out-File -Append init.coe
