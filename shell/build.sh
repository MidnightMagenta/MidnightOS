cd gnu-efi
make bootloader
echo making bootloader
cd ../kernel
make kernel
echo making kernel
make buildimg
echo making disk image
cd ..