start_time=$(date +%s%3N)

cd gnu-efi
make -j6
make bootloader -j6
cd ..
cd kernel
make -j6
make image
cd ..
end_time=$(date +%s%3N)
elapsed_time=$((end_time - start_time))

echo -e "\e[32m\e[1mBuild finished! Took: $elapsed_time ms\n\e[0m"