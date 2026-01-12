## to run this test file : chmod +x run_all_tests.sh

## !/bin/bash
for i in 1 2 3; 

do
    echo "****** Test Case $i ******"
    ./defrag images_frag/disk_frag_$i
    python3 diff_scripts/diff.py disk_defrag images_defrag/disk_defrag_$i
    echo
done
