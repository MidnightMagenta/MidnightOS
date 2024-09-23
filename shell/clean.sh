CLEANIMG = "$1"

cd gnu-efi/bootloader
make clean
cd ../../
cd kernel
make clean
if [ "$1" == "true" ]; then
    make cleanImage
fi
cd ..