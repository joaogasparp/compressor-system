import React, { useState } from 'react';
import {
  Typography,
  Button,
  FormControl,
  InputLabel,
  Select,
  MenuItem,
  Box,
  Alert,
  CircularProgress,
  Chip,
  Grid,
  ToggleButton,
  ToggleButtonGroup,
} from '@mui/material';
import {
  PlayArrow as PlayIcon,
  Download as DownloadIcon,
  Compress as CompressIcon,
  UnfoldMore as DecompressIcon,
} from '@mui/icons-material';

const CompressionDashboard = ({ file, onCompressionComplete, results }) => {
  const [algorithm, setAlgorithm] = useState('huffman');
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState('');
  const [mode, setMode] = useState('compress'); // 'compress' or 'decompress'

  const algorithms = [
    { value: 'rle', label: 'RLE', description: 'Run Length Encoding' },
    { value: 'huffman', label: 'Huffman', description: 'Optimal prefix codes' },
    { value: 'lz77', label: 'LZ77', description: 'Dictionary compression' },
  ];

  const handleOperation = async () => {
    if (!file) {
      setError('Please upload a file first');
      return;
    }

    setLoading(true);
    setError('');

    try {
      const formData = new FormData();
      formData.append('file', file);
      formData.append('algorithm', algorithm);

      const endpoint = mode === 'compress' ? '/compress' : '/decompress';
      const response = await fetch(endpoint, {
        method: 'POST',
        body: formData,
      });

      if (!response.ok) {
        throw new Error(`HTTP ${response.status}: ${response.statusText}`);
      }

      const responseText = await response.text();
      console.log('Raw response:', responseText); // Debug

      let data;
      try {
        data = JSON.parse(responseText);
      } catch (jsonError) {
        console.error('JSON Parse Error:', jsonError);
        console.error('Response was:', responseText);
        throw new Error('Invalid response format from server');
      }

      if (data.success) {
        // Convert base64 back to blob for download
        const binaryString = atob(data[mode === 'compress' ? 'compressed_data' : 'decompressed_data']);
        const bytes = new Uint8Array(binaryString.length);
        for (let i = 0; i < binaryString.length; i++) {
          bytes[i] = binaryString.charCodeAt(i);
        }
        const resultBlob = new Blob([bytes], { type: 'application/octet-stream' });

        const operationResults = {
          algorithm: data.algorithm.toUpperCase(),
          originalSize: mode === 'compress' ? data.original_size : data.compressed_size,
          processedSize: mode === 'compress' ? data.compressed_size : data.decompressed_size,
          ratio: mode === 'compress' ? 
            (data.compression_ratio * 100).toFixed(2) : 
            ((data.compressed_size / data.decompressed_size) * 100).toFixed(2),
          time: data[mode === 'compress' ? 'compression_time_ms' : 'decompression_time_ms'],
          verified: data.verified || true,
          resultFile: resultBlob,
          mode: mode,
        };

        onCompressionComplete(operationResults);
      } else {
        setError(`${mode} error: ` + (data.error || 'Unknown error'));
      }
    } catch (err) {
      setError(`Error during ${mode}: ` + err.message);
    } finally {
      setLoading(false);
    }
  };

  const handleDownload = () => {
    if (!results?.resultFile) return;

    const url = window.URL.createObjectURL(new Blob([results.resultFile]));
    const link = document.createElement('a');
    link.href = url;
    const suffix = results.mode === 'compress' ? 'compressed' : 'decompressed';
    link.download = `${file.name}.${algorithm}.${suffix}`;
    document.body.appendChild(link);
    link.click();
    link.remove();
    window.URL.revokeObjectURL(url);
  };

  const formatFileSize = (bytes) => {
    if (bytes === 0) return '0 Bytes';
    const k = 1024;
    const sizes = ['Bytes', 'KB', 'MB', 'GB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
  };

  return (
    <Box sx={{ 
      p: 3, 
      height: '100%',
      borderRadius: 8,
      backgroundColor: '#ffffff',
      border: '1px solid #e0e0e0',
      boxShadow: 'none !important',
    }}>
      <Typography variant="h6" gutterBottom sx={{ fontWeight: 500, mb: 3 }}>
        Processing Options
      </Typography>

      <Box sx={{ mb: 3 }}>
        <Typography variant="body2" sx={{ mb: 2, fontWeight: 500, color: 'text.primary' }}>
          Operation Mode
        </Typography>
        <ToggleButtonGroup
          value={mode}
          exclusive
          onChange={(e, newMode) => newMode && setMode(newMode)}
          fullWidth
          sx={{ 
            mb: 2,
            '& .MuiToggleButton-root': {
              py: 1.5,
              fontSize: '0.95rem',
              background: 'linear-gradient(135deg, rgba(255,255,255,0.9) 0%, rgba(255,255,255,0.7) 100%)',
              backdropFilter: 'blur(10px)',
              WebkitBackdropFilter: 'blur(10px)',
              border: '1px solid rgba(0,0,0,0.1)',
              '&.Mui-selected': {
                background: 'linear-gradient(135deg, rgba(0,0,0,0.85) 0%, rgba(0,0,0,0.7) 100%)',
                color: '#ffffff',
                border: '1px solid rgba(255,255,255,0.2)',
              },
            },
          }}
        >
          <ToggleButton value="compress">
            <CompressIcon sx={{ mr: 1 }} />
            Compress
          </ToggleButton>
          <ToggleButton value="decompress">
            <DecompressIcon sx={{ mr: 1 }} />
            Decompress
          </ToggleButton>
        </ToggleButtonGroup>
      </Box>

      <Box sx={{ mb: 3 }}>
        <FormControl fullWidth>
          <InputLabel>Algorithm</InputLabel>
          <Select
            value={algorithm}
            label="Algorithm"
            onChange={(e) => setAlgorithm(e.target.value)}
            disabled={loading}
          >
            {algorithms.map((algo) => (
              <MenuItem key={algo.value} value={algo.value}>
                <Box>
                  <Typography variant="body1" sx={{ fontWeight: 500 }}>
                    {algo.label}
                  </Typography>
                  <Typography variant="caption" color="text.secondary">
                    {algo.description}
                  </Typography>
                </Box>
              </MenuItem>
            ))}
          </Select>
        </FormControl>
      </Box>

      {error && (
        <Alert severity="error" sx={{ mb: 2 }}>
          {error}
        </Alert>
      )}

      <Box sx={{ mb: 3 }}>
        <Button
          variant="contained"
          fullWidth
          onClick={handleOperation}
          disabled={!file || loading}
          startIcon={loading ? <CircularProgress size={20} color="inherit" /> : <PlayIcon />}
          size="large"
        >
          {loading ? `${mode}ing...` : `${mode === 'compress' ? 'Compress' : 'Decompress'} File`}
        </Button>
      </Box>

      {results && (
        <Box sx={{ 
          p: 3, 
          border: '1px solid #e0e0e0', 
          borderRadius: 8,
          backgroundColor: '#ffffff',
          boxShadow: '0 2px 8px rgba(0,0,0,0.1)',
        }}>
          <Typography variant="subtitle1" gutterBottom sx={{ 
            fontWeight: 600, 
            mb: 2,
            color: 'text.primary',
          }}>
            {results.mode === 'compress' ? 'Compression' : 'Decompression'} Results
          </Typography>

          <Grid container spacing={2} sx={{ mb: 3 }}>
            <Grid item xs={6}>
              <Box sx={{ 
                textAlign: 'center', 
                p: 2,
                border: '1px solid #e0e0e0',
                borderRadius: 6,
                backgroundColor: '#f9f9f9',
              }}>
                <Typography variant="caption" color="text.secondary" sx={{ fontWeight: 500 }}>
                  {results.mode === 'compress' ? 'Original' : 'Compressed'}
                </Typography>
                <Typography variant="h6" sx={{ fontWeight: 600, mt: 0.5 }}>
                  {formatFileSize(results.originalSize)}
                </Typography>
              </Box>
            </Grid>
            <Grid item xs={6}>
              <Box sx={{ 
                textAlign: 'center', 
                p: 2,
                border: '1px solid #e0e0e0',
                borderRadius: 6,
                backgroundColor: '#f9f9f9',
              }}>
                <Typography variant="caption" color="text.secondary" sx={{ fontWeight: 500 }}>
                  {results.mode === 'compress' ? 'Compressed' : 'Decompressed'}
                </Typography>
                <Typography variant="h6" sx={{ fontWeight: 600, mt: 0.5 }}>
                  {formatFileSize(results.processedSize)}
                </Typography>
              </Box>
            </Grid>
          </Grid>

          <Box sx={{ display: 'flex', gap: 1, flexWrap: 'wrap', mb: 2, justifyContent: 'center' }}>
            <Chip
              label={`Ratio: ${results.ratio}%`}
              size="small"
              variant="outlined"
            />
            <Chip
              label={`Time: ${results.time}ms`}
              size="small"
              variant="outlined"
            />
            <Chip
              label={results.verified ? 'Verified' : 'Error'}
              color={results.verified ? 'success' : 'error'}
              size="small"
            />
          </Box>

          <Button
            variant="outlined"
            fullWidth
            onClick={handleDownload}
            startIcon={<DownloadIcon />}
            disabled={!results.resultFile}
          >
            Download {results.mode === 'compress' ? 'Compressed' : 'Decompressed'} File
          </Button>
        </Box>
      )}
    </Box>
  );
};

export default CompressionDashboard;
