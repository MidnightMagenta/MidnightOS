start_time=$(date +%s%3N)

cd gnu-efi
make bootloader
cd ..
cd kernel
make
make image
cd ..

end_time=$(date +%s%3N)
elapsed_time=$((end_time - start_time))

echo -e "\e[32m\e[1mBuild finished! Took: $elapsed_time ms\n\e[0m"