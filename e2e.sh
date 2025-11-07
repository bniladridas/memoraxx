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
    if command -v gtimeout >/dev/null 2>&1; then
        TIMEOUT_CMD="gtimeout"
    elif command -v timeout >/dev/null 2>&1; then
        TIMEOUT_CMD="timeout"
    else
        echo "Warning: timeout command not found. Running without timeout."
        TIMEOUT_CMD=""
    fi

    if [ -n "$TIMEOUT_CMD" ]; then
        $TIMEOUT_CMD 30s bash -c 'echo "Hello, test message" | ./build/memoraxx' > test_output.txt 2>&1 &
    else
        bash -c 'echo "Hello, test message" | ./build/memoraxx' > test_output.txt 2>&1 &
    fi
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

# Test executable exists and is executable
if [ -x "./build/memoraxx" ]; then
    echo "✓ Executable test passed"
else
    echo "✗ Executable test failed"
    exit 1
fi

# Test config loading
echo "Testing config loading..."
cat > test_config.json <<EOF
{
    "base_url": "http://localhost:11434/api/generate",
    "model": "llama3.2",
    "max_tokens": 2048,
    "memory_file": "test_memory.json"
}
EOF

bash -c 'echo "exit" | ./build/memoraxx' > config_test.txt 2>&1 &
CONFIG_PID=$!
wait $CONFIG_PID 2>/dev/null || true

if [ -f "test_memory.json" ]; then
    echo "✓ Config and memory persistence test passed"
    rm test_memory.json
else
    echo "✗ Config and memory persistence test failed"
    exit 1
fi

rm test_config.json config_test.txt

# Test commands
echo "Testing commands..."
bash -c 'echo -e "test message\nclear\nexit" | ./build/memoraxx' > command_test.txt 2>&1 &
CMD_PID=$!
wait $CMD_PID 2>/dev/null || true

if grep -q "Memory cleared" command_test.txt; then
    echo "✓ Command test passed"
else
    echo "✗ Command test failed"
    cat command_test.txt
    exit 1
fi

rm command_test.txt

echo "All E2E tests completed successfully!"
