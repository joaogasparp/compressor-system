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
  Card,
  CardContent,
  LinearProgress,
  Fade,
  Zoom,
} from '@mui/material';
import {
  PlayArrow as PlayIcon,
  Download as DownloadIcon,
  Speed as SpeedIcon,
  Compress as CompressIcon,
  Assessment as AssessmentIcon,
  Timer as TimerIcon,
  CheckCircle as CheckIcon,
  Error as ErrorIcon,
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
    <Paper 
      sx={{ 
        p: 4, 
        height: '100%',
        background: 'linear-gradient(135deg, rgba(255,255,255,0.9) 0%, rgba(245,247,250,0.9) 100%)',
        backdropFilter: 'blur(20px)',
        borderRadius: 3,
        border: '1px solid rgba(255,255,255,0.3)',
        boxShadow: '0 8px 32px rgba(0,0,0,0.1)',
      }}
    >
      <Box sx={{ display: 'flex', alignItems: 'center', mb: 3 }}>
        <AssessmentIcon sx={{ mr: 2, color: 'primary.main', fontSize: 28 }} />
        <Typography 
          variant="h5" 
          sx={{ 
            fontWeight: 600,
            background: 'linear-gradient(45deg, #1976d2, #42a5f5)',
            backgroundClip: 'text',
            WebkitBackgroundClip: 'text',
            WebkitTextFillColor: 'transparent',
          }}
        >
          Compression Dashboard
        </Typography>
      </Box>

      <Card sx={{ 
        mb: 3, 
        background: 'rgba(255,255,255,0.7)',
        backdropFilter: 'blur(10px)',
        borderRadius: 2,
        border: '1px solid rgba(255,255,255,0.5)',
      }}>
        <CardContent>
          <FormControl fullWidth>
            <InputLabel sx={{ fontWeight: 500 }}>Compression Algorithm</InputLabel>
            <Select
              value={algorithm}
              label="Compression Algorithm"
              onChange={(e) => setAlgorithm(e.target.value)}
              disabled={loading}
              sx={{
                '& .MuiOutlinedInput-root': {
                  borderRadius: 2,
                  '&:hover .MuiOutlinedInput-notchedOutline': {
                    borderColor: 'primary.main',
                  },
                },
              }}
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
        </CardContent>
      </Card>

      {error && (
        <Fade in={!!error}>
          <Alert 
            severity="error" 
            sx={{ 
              mb: 2,
              borderRadius: 2,
              '& .MuiAlert-icon': {
                fontSize: 24,
              },
            }}
          >
            {error}
          </Alert>
        </Fade>
      )}

      <Box sx={{ mb: 3 }}>
        <Button
          variant="contained"
          fullWidth
          onClick={handleCompress}
          disabled={!file || loading}
          startIcon={loading ? <CircularProgress size={20} color="inherit" /> : <PlayIcon />}
          size="large"
          sx={{
            py: 2,
            borderRadius: 2,
            fontSize: '1.1rem',
            fontWeight: 600,
            textTransform: 'none',
            background: loading 
              ? 'linear-gradient(45deg, #90caf9, #64b5f6)' 
              : 'linear-gradient(45deg, #1976d2, #42a5f5)',
            '&:hover': {
              background: 'linear-gradient(45deg, #1565c0, #1976d2)',
              transform: 'translateY(-2px)',
              boxShadow: '0 8px 25px rgba(25,118,210,0.3)',
            },
            '&:disabled': {
              background: 'linear-gradient(45deg, #e0e0e0, #bdbdbd)',
            },
            transition: 'all 0.3s cubic-bezier(0.4, 0, 0.2, 1)',
          }}
        >
          {loading ? 'Compressing...' : 'Compress File'}
        </Button>
        {loading && (
          <LinearProgress 
            sx={{ 
              mt: 1, 
              borderRadius: 1,
              height: 6,
              '& .MuiLinearProgress-bar': {
                borderRadius: 1,
              },
            }} 
          />
        )}
      </Box>

      {results && (
        <Zoom in={!!results}>
          <Card sx={{
            background: 'linear-gradient(135deg, rgba(76,175,80,0.1) 0%, rgba(139,195,74,0.05) 100%)',
            backdropFilter: 'blur(10px)',
            borderRadius: 3,
            border: '1px solid rgba(76,175,80,0.3)',
            overflow: 'hidden',
          }}>
            <CardContent sx={{ p: 3 }}>
              <Box sx={{ display: 'flex', alignItems: 'center', mb: 3 }}>
                <CheckIcon sx={{ mr: 2, color: 'success.main', fontSize: 28 }} />
                <Typography 
                  variant="h6" 
                  sx={{ 
                    fontWeight: 600,
                    color: 'success.dark',
                  }}
                >
                  Compression Analysis
                </Typography>
              </Box>

              <Grid container spacing={3} sx={{ mb: 3 }}>
                <Grid item xs={6}>
                  <Card sx={{ 
                    textAlign: 'center', 
                    p: 2,
                    background: 'rgba(255,255,255,0.8)',
                    borderRadius: 2,
                    border: '1px solid rgba(255,255,255,0.5)',
                  }}>
                    <CompressIcon sx={{ color: 'primary.main', fontSize: 32, mb: 1 }} />
                    <Typography variant="body2" color="text.secondary" sx={{ fontWeight: 500 }}>
                      Original Size
                    </Typography>
                    <Typography variant="h5" sx={{ fontWeight: 700, color: 'primary.main' }}>
                      {formatFileSize(results.originalSize)}
                    </Typography>
                  </Card>
                </Grid>
                <Grid item xs={6}>
                  <Card sx={{ 
                    textAlign: 'center', 
                    p: 2,
                    background: 'rgba(255,255,255,0.8)',
                    borderRadius: 2,
                    border: '1px solid rgba(255,255,255,0.5)',
                  }}>
                    <SpeedIcon sx={{ color: 'secondary.main', fontSize: 32, mb: 1 }} />
                    <Typography variant="body2" color="text.secondary" sx={{ fontWeight: 500 }}>
                      Compressed Size
                    </Typography>
                    <Typography variant="h5" sx={{ fontWeight: 700, color: 'secondary.main' }}>
                      {formatFileSize(results.compressedSize)}
                    </Typography>
                  </Card>
                </Grid>
              </Grid>

              <Box sx={{ display: 'flex', gap: 2, flexWrap: 'wrap', mb: 3, justifyContent: 'center' }}>
                <Chip
                  icon={<CompressIcon />}
                  label={`Savings: ${(100 - parseFloat(results.ratio)).toFixed(1)}%`}
                  color="success"
                  variant="filled"
                  sx={{ 
                    fontWeight: 600,
                    fontSize: '0.9rem',
                    px: 2,
                    py: 1,
                    borderRadius: 2,
                  }}
                />
                <Chip
                  icon={<TimerIcon />}
                  label={`Time: ${results.time}ms`}
                  color="info"
                  variant="outlined"
                  sx={{ 
                    fontWeight: 600,
                    fontSize: '0.9rem',
                    px: 2,
                    py: 1,
                    borderRadius: 2,
                    borderWidth: 2,
                  }}
                />
                <Chip
                  icon={results.verified ? <CheckIcon /> : <ErrorIcon />}
                  label={results.verified ? 'Verified ✓' : 'Error ✗'}
                  color={results.verified ? 'success' : 'error'}
                  variant="outlined"
                  sx={{ 
                    fontWeight: 600,
                    fontSize: '0.9rem',
                    px: 2,
                    py: 1,
                    borderRadius: 2,
                    borderWidth: 2,
                  }}
                />
              </Box>

              <Button
                variant="outlined"
                fullWidth
                onClick={handleDownload}
                startIcon={<DownloadIcon />}
                disabled={!results.compressedFile}
                size="large"
                sx={{
                  py: 2,
                  borderRadius: 2,
                  fontSize: '1.1rem',
                  fontWeight: 600,
                  textTransform: 'none',
                  borderWidth: 2,
                  '&:hover': {
                    borderWidth: 2,
                    transform: 'translateY(-2px)',
                    boxShadow: '0 8px 25px rgba(76,175,80,0.3)',
                  },
                  transition: 'all 0.3s cubic-bezier(0.4, 0, 0.2, 1)',
                }}
              >
                Download Compressed File
              </Button>
            </CardContent>
          </Card>
        </Zoom>
      )}
    </Paper>
  );
};

export default CompressionDashboard;
