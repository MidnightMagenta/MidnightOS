start_time=$(date +%s%3N)

cd gnu-efi
make bootloader -j6
cd ..
cd kernel
make
if [ -e bin/MidnightOS.img ]; then
    echo -e "\e[32mDisk image already exists. Skipping image creation\e[0m"
else
	echo -e "\e[31mMaking disk image\e[0m"
	make makeImage
fi

make image
cd ..

end_time=$(date +%s%3N)
elapsed_time=$((end_time - start_time))

echo -e "\e[32m\e[1mBuild finished! Took: $elapsed_time ms\n\e[0m"