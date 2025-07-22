import React, { useState } from 'react';
import { 
  Container, 
  Grid, 
  Paper, 
  Typography, 
  Box, 
  ThemeProvider, 
  createTheme,
  CssBaseline 
} from '@mui/material';
import FileUpload from './FileUpload';
import CompressionDashboard from './CompressionDashboard';

const theme = createTheme({
  palette: {
    mode: 'light',
    primary: {
      main: '#2563eb', // Modern blue
      light: '#60a5fa',
      dark: '#1d4ed8',
    },
    secondary: {
      main: '#7c3aed', // Modern purple
      light: '#a78bfa',
      dark: '#5b21b6',
    },
    background: {
      default: '#f8fafc',
      paper: '#ffffff',
    },
    success: {
      main: '#10b981',
      light: '#34d399',
      dark: '#059669',
    },
    error: {
      main: '#ef4444',
      light: '#f87171',
      dark: '#dc2626',
    },
  },
  typography: {
    fontFamily: '"Inter", "Roboto", "Helvetica", "Arial", sans-serif',
    h4: {
      fontWeight: 700,
      letterSpacing: '-0.025em',
    },
    h6: {
      fontWeight: 600,
    },
    body1: {
      lineHeight: 1.6,
    },
  },
  shape: {
    borderRadius: 12,
  },
  components: {
    MuiPaper: {
      styleOverrides: {
        root: {
          backdropFilter: 'blur(10px)',
          border: '1px solid rgba(255, 255, 255, 0.1)',
        },
      },
    },
    MuiButton: {
      styleOverrides: {
        root: {
          textTransform: 'none',
          fontWeight: 600,
          boxShadow: 'none',
          '&:hover': {
            boxShadow: 'none',
          },
        },
      },
    },
  },
});

function App() {
  const [selectedFile, setSelectedFile] = useState(null);
  const [compressionResults, setCompressionResults] = useState(null);

  const handleFileSelect = (file) => {
    setSelectedFile(file);
    setCompressionResults(null);
  };

  const handleCompressionComplete = (results) => {
    setCompressionResults(results);
  };

  return (
    <ThemeProvider theme={theme}>
      <CssBaseline />
      <Box sx={{ 
        minHeight: '100vh',
        background: 'linear-gradient(135deg, #667eea 0%, #764ba2 100%)',
        py: 4
      }}>
        <Container maxWidth="lg">
          {/* Header */}
          <Paper 
            elevation={0} 
            sx={{ 
              p: 4, 
              mb: 4, 
              background: 'rgba(255, 255, 255, 0.95)',
              backdropFilter: 'blur(10px)',
              borderRadius: 3,
              textAlign: 'center'
            }}
          >
            <Typography 
              variant="h4" 
              component="h1" 
              gutterBottom 
              sx={{ 
                background: 'linear-gradient(45deg, #2563eb, #7c3aed)',
                backgroundClip: 'text',
                WebkitBackgroundClip: 'text',
                WebkitTextFillColor: 'transparent',
                mb: 2
              }}
            >
              Advanced File Compression
            </Typography>
            <Typography variant="h6" color="text.secondary" sx={{ mb: 1 }}>
              Compress files using state-of-the-art algorithms
            </Typography>
            <Typography variant="body2" color="text.secondary">
              Choose from LZ77, Huffman, or RLE compression methods
            </Typography>
          </Paper>

          {/* Main Content */}
          <Grid container spacing={3}>
            <Grid item xs={12} md={6}>
              <FileUpload 
                onFileSelect={handleFileSelect} 
                selectedFile={selectedFile}
              />
            </Grid>
            <Grid item xs={12} md={6}>
              <CompressionDashboard 
                file={selectedFile}
                onCompressionComplete={handleCompressionComplete}
                results={compressionResults}
              />
            </Grid>
          </Grid>

          {/* Results Summary */}
          {compressionResults && (
            <Box sx={{ mt: 3 }}>
              <Paper 
                sx={{ 
                  p: 4,
                  background: 'rgba(255, 255, 255, 0.95)',
                  backdropFilter: 'blur(10px)',
                  borderRadius: 3
                }}
              >
                <Typography variant="h6" gutterBottom sx={{ mb: 3 }}>
                  Compression Results
                </Typography>
                <Grid container spacing={3}>
                  <Grid item xs={12} sm={6} md={3}>
                    <Box sx={{ textAlign: 'center' }}>
                      <Typography variant="h4" color="primary" sx={{ fontWeight: 700 }}>
                        {compressionResults.algorithm}
                      </Typography>
                      <Typography variant="body2" color="text.secondary">
                        Algorithm Used
                      </Typography>
                    </Box>
                  </Grid>
                  <Grid item xs={12} sm={6} md={3}>
                    <Box sx={{ textAlign: 'center' }}>
                      <Typography variant="h4" color="success.main" sx={{ fontWeight: 700 }}>
                        {(compressionResults.originalSize / 1024).toFixed(1)} KB
                      </Typography>
                      <Typography variant="body2" color="text.secondary">
                        Original Size
                      </Typography>
                    </Box>
                  </Grid>
                  <Grid item xs={12} sm={6} md={3}>
                    <Box sx={{ textAlign: 'center' }}>
                      <Typography variant="h4" color="secondary.main" sx={{ fontWeight: 700 }}>
                        {(compressionResults.compressedSize / 1024).toFixed(1)} KB
                      </Typography>
                      <Typography variant="body2" color="text.secondary">
                        Compressed Size
                      </Typography>
                    </Box>
                  </Grid>
                  <Grid item xs={12} sm={6} md={3}>
                    <Box sx={{ textAlign: 'center' }}>
                      <Typography variant="h4" color="error.main" sx={{ fontWeight: 700 }}>
                        {compressionResults.ratio}%
                      </Typography>
                      <Typography variant="body2" color="text.secondary">
                        Final Ratio
                      </Typography>
                    </Box>
                  </Grid>
                </Grid>
                
                <Box sx={{ mt: 3, display: 'flex', justifyContent: 'center', gap: 2 }}>
                  <Typography variant="body2" color="text.secondary">
                    Processing time: <strong>{compressionResults.time} ms</strong>
                  </Typography>
                  <Typography variant="body2" color="text.secondary">
                    •
                  </Typography>
                  <Typography variant="body2" color="text.secondary">
                    Integrity: <strong>{compressionResults.verified ? 'Verified' : 'Error'}</strong>
                  </Typography>
                </Box>
              </Paper>
            </Box>
          )}
        </Container>
      </Box>
    </ThemeProvider>
  );
}

export default App;
