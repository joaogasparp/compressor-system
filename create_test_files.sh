# Sample test files for the Compressor System

# Text file with repetitive content (good for RLE)
echo "Creating low-entropy test file..."
python3 -c "
content = 'AAAAAAAAAA' * 100 + 'BBBBBBBBBB' * 100 + 'CCCCCCCCCC' * 100
with open('test/sample-files/low_entropy.txt', 'w') as f:
    f.write(content)
print(f'Created low_entropy.txt ({len(content)} bytes)')
"

# Random binary data (good for Huffman)
echo "Creating high-entropy test file..."
python3 -c "
import random
data = bytes([random.randint(0, 255) for _ in range(10000)])
with open('test/sample-files/high_entropy.bin', 'wb') as f:
    f.write(data)
print(f'Created high_entropy.bin ({len(data)} bytes)')
"

# Structured text with patterns (good for LZ77)
echo "Creating structured test file..."
python3 -c "
content = '''
The quick brown fox jumps over the lazy dog.
The quick brown fox jumps over the lazy dog.
This sentence repeats multiple times.
This sentence repeats multiple times.
This sentence repeats multiple times.
Lorem ipsum dolor sit amet, consectetur adipiscing elit.
Lorem ipsum dolor sit amet, consectetur adipiscing elit.
Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
''' * 50
with open('test/sample-files/structured.txt', 'w') as f:
    f.write(content)
print(f'Created structured.txt ({len(content)} bytes)')
"

# Mixed content (good for hybrid algorithm)
echo "Creating mixed content test file..."
python3 -c "
import random
content = ''
# Add some repetitive sections
content += 'A' * 1000
content += 'Hello World! ' * 200
# Add some random text
content += ''.join(chr(random.randint(65, 90)) for _ in range(2000))
# Add structured patterns
content += 'Pattern123Pattern123Pattern123' * 100
with open('test/sample-files/mixed.txt', 'w') as f:
    f.write(content)
print(f'Created mixed.txt ({len(content)} bytes)')
"

echo "Sample test files created in test/sample-files/"
