#include <windows.h>
#include <iostream>

int main()
{
    DWORD size = 10000;
    DWORD pattern_size = 6;
    DWORD base = 0x1000;
    DWORD i = 0;
    
    // Mock VirtualQuery
    // Let's say region 1: size 4000, COMMIT
    // region 2: size 2000, NOACCESS
    // region 3: size 4000, COMMIT
    
    int loops = 0;
    for (; i < size - pattern_size; )
    {
        if (loops++ > 100) { std::cout << "Infinite loop!\n"; break; }
        
        DWORD region_size = 4000;
        if (i >= 4000 && i < 6000) region_size = 6000 - i;
        else if (i >= 6000) region_size = 10000 - i;
        else region_size = 4000 - i;
        
        bool is_commit = (i < 4000 || i >= 6000);
        
        if (!is_commit)
        {
            i += region_size;
            std::cout << "Skipped to " << i << "\n";
            continue;
        }

        DWORD region_end = i + region_size;
        if (region_end > size) region_end = size;
        if (region_end < pattern_size) break;

        std::cout << "Scanning " << i << " to " << region_end - pattern_size << "\n";
        for (; i <= region_end - pattern_size; ++i)
        {
            // scan...
        }
        std::cout << "Inner loop ended at i = " << i << "\n";
        
        i = region_end;
        std::cout << "i set to " << i << "\n";
    }
    std::cout << "Finished!\n";
    return 0;
}
