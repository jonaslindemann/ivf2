/**
 * @file optimized_cursor_example.cpp
 * @brief Example demonstrating efficient cursor position updates with proper ground projection
 * 
 * This example shows how to use the optimized Cursor class with efficient
 * position updates that avoid costly mesh rebuilds and correctly handle
 * the ground projection without double transformation issues.
 */

#include <ivf/cursor.h>
#include <glm/glm.hpp>
#include <iostream>
#include <iomanip>

using namespace ivf;

void demonstrateOptimizedCursor()
{
    // Create a cursor with ground projection enabled
    auto cursor = Cursor::create(1.0f, 0.1f, true);
    
    std::cout << "=== Optimized Cursor Usage Example ===" << std::endl;
    
    // Initial setup - this will do a full mesh rebuild (unavoidable first time)
    std::cout << "1. Initial cursor setup at origin" << std::endl;
    glm::vec3 initialPos(0.0f, 2.0f, 0.0f);
    cursor->updatePosition(initialPos);
    std::cout << "   Cursor at: (" << initialPos.x << ", " << initialPos.y << ", " << initialPos.z << ")" << std::endl;
    std::cout << "   Ground projection: line from cursor to ground (0, 0, 0)" << std::endl;
    
    // Simulate frequent position updates (e.g., from mouse movement)
    // These will be highly optimized - only updating ground projection vertices
    std::cout << "\n2. Performing optimized position updates..." << std::endl;
    
    for (int i = 0; i < 10; ++i)
    {
        // Simulate cursor movement (e.g., following mouse in 3D space)
        glm::vec3 newPos(
            std::sin(i * 0.2f) * 2.0f,      // X movement: -2 to +2
            3.0f + std::cos(i * 0.15f),     // Y movement: 2 to 4  
            std::cos(i * 0.2f) * 2.0f       // Z movement: -2 to +2
        );
        
        // This call is highly optimized - only updates ground projection line vertices
        cursor->updatePosition(newPos);
        
        std::cout << "   Update " << i << ": Position(" 
                  << std::fixed << std::setprecision(2)
                  << newPos.x << ", " << newPos.y << ", " << newPos.z << ")" << std::endl;
        std::cout << "      ? Ground projection to (" << newPos.x << ", 0.0, " << newPos.z << ")" << std::endl;
    }
    
    // Test edge cases
    std::cout << "\n3. Testing edge cases..." << std::endl;
    
    // Cursor at ground level
    cursor->updatePosition(glm::vec3(1.0f, 0.0f, 1.0f));
    std::cout << "   Cursor at ground level (1, 0, 1) - projection line should be minimal" << std::endl;
    
    // Cursor below ground (should still work)
    cursor->updatePosition(glm::vec3(-1.0f, -2.0f, -1.0f));
    std::cout << "   Cursor below ground (-1, -2, -1) - projection extends upward to ground" << std::endl;
    
    // Very high cursor
    cursor->updatePosition(glm::vec3(0.0f, 10.0f, 0.0f));
    std::cout << "   High cursor (0, 10, 0) - long projection line to ground" << std::endl;
    
    // Demonstrate when full rebuild is needed
    std::cout << "\n4. Operations that trigger full rebuild..." << std::endl;
    cursor->setSize(2.0f); // This will trigger a full mesh rebuild
    std::cout << "   Size changed to 2.0 (full rebuild triggered)" << std::endl;
    
    cursor->setGap(0.5f); // This will only update ground projection vertices
    std::cout << "   Gap changed to 0.5 (optimized update)" << std::endl;
    
    // Position updates after changes are still optimized
    std::cout << "\n5. Position updates after changes (still optimized)..." << std::endl;
    cursor->updatePosition(glm::vec3(5.0f, 3.0f, 2.0f));
    std::cout << "   Final position: (5, 3, 2) with size=2.0, gap=0.5" << std::endl;
    
    std::cout << "\n=== Performance Summary ===" << std::endl;
    std::cout << "? FIXED: Ground projection now uses local coordinates" << std::endl;
    std::cout << "? FIXED: No double transformation (TransformNode + vertex updates)" << std::endl;
    std::cout << "? OPTIMIZED: Position changes only update 2 vertices" << std::endl;
    std::cout << "? EFFICIENT: Full rebuild only when geometry actually changes" << std::endl;
    
    std::cout << "\nTriggers for full rebuild:" << std::endl;
    std::cout << "  - setSize() - changes axis length" << std::endl;
    std::cout << "  - setShowGroundProjection() - adds/removes projection line" << std::endl;
    std::cout << "  - setAxisColors() - changes vertex colors" << std::endl;
    std::cout << "  - setUseCustomColors() - toggles color system" << std::endl;
    
    std::cout << "\nOptimized operations:" << std::endl;
    std::cout << "  - updatePosition() - only updates projection vertices" << std::endl;
    std::cout << "  - setGap() - only updates projection vertices (when enabled)" << std::endl;
}

int main()
{
    demonstrateOptimizedCursor();
    return 0;
}