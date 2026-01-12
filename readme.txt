====================================
Project 4: File System Defragmenter
====================================

# COMPILATION

    make clean && make 

This produces the executable: defrag


# RUN / TESTING

Note: To make the test file executable , run : chmod +x run_all_tests.sh 

---------------------------------------------------------------------------------------------------
Option 1 - Run all tests:       |   Option 2 - Test individual image:
---------------------------------------------------------------------------------------------------
    ./run_all_tests.sh          |   ./defrag images_frag/disk_frag_1
                                |   cd diff_scripts/
                                |   python3 diff.py ../disk_defrag ../images_defrag/disk_defrag_1
---------------------------------------------------------------------------------------------------

# EXECUTION

    ./defrag <input_disk_image>

Example:
    ./defrag images_frag/disk_frag_1

Output file: disk_defrag


Expected output: 
    === Test Case 1 ===
    Input: disk_defrag
    Expected: images_defrag/disk_defrag_1
    Match: 1.0

    === Test Case 2 ===
    Input: disk_defrag
    Expected: images_defrag/disk_defrag_2
    Match: 1.0

    === Test Case 3 ===
    Input: disk_defrag
    Expected: images_defrag/disk_defrag_3
    Match: 1.0

