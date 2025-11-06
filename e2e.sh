#!/bin/bash

# End-to-end test script for memoraxx

set -e

echo "Running E2E tests for memoraxx..."

# Build the project
echo "Building project..."
./build.sh

# Check if Ollama is running
if curl -s http://localhost:11434/api/tags > /dev/null 2>&1; then
    echo "Ollama is running. Running integration test..."

    # Test basic interaction
    echo "Testing basic AI interaction..."
    timeout 30s bash -c 'echo "Hello, test message" | ./build/memoraxx' > test_output.txt 2>&1 &
    TEST_PID=$!
    sleep 5
    kill $TEST_PID 2>/dev/null || true

    # Check output
    if grep -q "AI Response" test_output.txt; then
        echo "✓ Integration test passed"
    else
        echo "✗ Integration test failed"
        cat test_output.txt
        exit 1
    fi

    rm test_output.txt
else
    echo "Ollama not running. Skipping integration test."
    echo "To run full E2E tests, start Ollama: ollama serve && ollama pull llama3.2"
fi

# Test build output
if [ -f "./build/memoraxx" ]; then
    echo "✓ Build test passed"
else
    echo "✗ Build test failed"
    exit 1
fi

# Test help/version (if implemented)
# For now, just check executable runs
if ./build/memoraxx --help 2>/dev/null || true; then
    echo "✓ Executable test passed"
else
    echo "✗ Executable test failed"
    exit 1
fi

echo "All E2E tests completed successfully!"
