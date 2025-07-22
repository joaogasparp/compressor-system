import React, { useState } from 'react';
import {
  Paper,
  Typography,
  Button,
  FormControl,
  InputLabel,
  Select,
  MenuItem,
  Box,
  Alert,
  CircularProgress,
  Divider,
  Chip,
  Grid,
} from '@mui/material';
import {
  PlayArrow as PlayIcon,
  Download as DownloadIcon,
  Speed as SpeedIcon,
  Compress as CompressIcon,
} from '@mui/icons-material';

const CompressionDashboard = ({ file, onCompressionComplete, results }) => {
  const [algorithm, setAlgorithm] = useState('huffman');
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState('');

  const algorithms = [
    { value: 'rle', label: 'RLE (Run Length Encoding)', description: 'Best for repetitive data' },
    { value: 'huffman', label: 'Huffman', description: 'Best general compression' },
    { value: 'lz77', label: 'LZ77', description: 'Universal compression' },
  ];

  const handleCompress = async () => {
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

      const response = await fetch('/compress', {
        method: 'POST',
        body: formData,
      });

      const data = await response.json();

      if (data.success) {
        // Convert base64 back to blob for download
        const binaryString = atob(data.compressed_data);
        const bytes = new Uint8Array(binaryString.length);
        for (let i = 0; i < binaryString.length; i++) {
          bytes[i] = binaryString.charCodeAt(i);
        }
        const compressedBlob = new Blob([bytes], { type: 'application/octet-stream' });

        const compressionResults = {
          algorithm: data.algorithm.toUpperCase(),
          originalSize: data.original_size,
          compressedSize: data.compressed_size,
          ratio: (data.compression_ratio * 100).toFixed(2),
          time: data.compression_time_ms,
          verified: data.verified,
          compressedFile: compressedBlob,
        };

        onCompressionComplete(compressionResults);
      } else {
        setError('Compression error: ' + (data.error || 'Unknown error'));
      }
    } catch (err) {
      setError('Error during compression: ' + err.message);
    } finally {
      setLoading(false);
    }
  };

  const handleDownload = () => {
    if (!results?.compressedFile) return;

    const url = window.URL.createObjectURL(new Blob([results.compressedFile]));
    const link = document.createElement('a');
    link.href = url;
    link.download = `${file.name}.${algorithm}.compressed`;
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
    <Paper sx={{ p: 3, height: '100%' }}>
      <Typography variant="h6" gutterBottom>
        Compression Dashboard
      </Typography>

      <Box sx={{ mb: 3 }}>
        <FormControl fullWidth>
          <InputLabel>Compression Algorithm</InputLabel>
          <Select
            value={algorithm}
            label="Compression Algorithm"
            onChange={(e) => setAlgorithm(e.target.value)}
            disabled={loading}
          >
            {algorithms.map((algo) => (
              <MenuItem key={algo.value} value={algo.value}>
                <Box>
                  <Typography variant="body1">{algo.label}</Typography>
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
          onClick={handleCompress}
          disabled={!file || loading}
          startIcon={loading ? <CircularProgress size={20} /> : <PlayIcon />}
          size="large"
        >
          {loading ? 'Compressing...' : 'Compress File'}
        </Button>
      </Box>

      {results && (
        <Box>
          <Divider sx={{ mb: 2 }} />
          <Typography variant="subtitle1" gutterBottom>
            Compression Analysis
          </Typography>

          <Grid container spacing={2} sx={{ mb: 2 }}>
            <Grid item xs={6}>
              <Box sx={{ textAlign: 'center', p: 1 }}>
                <CompressIcon color="primary" />
                <Typography variant="body2" color="text.secondary">
                  Original
                </Typography>
                <Typography variant="h6">
                  {formatFileSize(results.originalSize)}
                </Typography>
              </Box>
            </Grid>
            <Grid item xs={6}>
              <Box sx={{ textAlign: 'center', p: 1 }}>
                <SpeedIcon color="secondary" />
                <Typography variant="body2" color="text.secondary">
                  Compressed
                </Typography>
                <Typography variant="h6">
                  {formatFileSize(results.compressedSize)}
                </Typography>
              </Box>
            </Grid>
          </Grid>

          <Box sx={{ display: 'flex', gap: 1, flexWrap: 'wrap', mb: 2 }}>
            <Chip
              label={`Savings: ${(100 - parseFloat(results.ratio)).toFixed(1)}%`}
              color="success"
              variant="filled"
            />
            <Chip
              label={`Time: ${results.time}ms`}
              color="info"
              variant="outlined"
            />
            <Chip
              label={results.verified ? 'Verified' : 'Error'}
              color={results.verified ? 'success' : 'error'}
              variant="outlined"
            />
          </Box>

          <Button
            variant="outlined"
            fullWidth
            onClick={handleDownload}
            startIcon={<DownloadIcon />}
            disabled={!results.compressedFile}
          >
            Download Compressed File
          </Button>
        </Box>
      )}
    </Paper>
  );
};

export default CompressionDashboard;
