import React, { useState } from 'react';
import { 
  ThemeProvider, 
  createTheme, 
  CssBaseline, 
  Container, 
  Typography, 
  Box, 
  Paper,
  Grid
} from '@mui/material';
import FileUpload from './FileUpload';
import CompressionDashboard from './CompressionDashboard';

// Minimal black and white theme
const theme = createTheme({
  palette: {
    mode: 'light',
    primary: {
      main: '#000000',
      light: '#333333',
      dark: '#000000',
    },
    secondary: {
      main: '#666666',
      light: '#888888',
      dark: '#444444',
    },
    background: {
      default: '#ffffff',
      paper: '#ffffff',
    },
    text: {
      primary: '#000000',
      secondary: '#666666',
    },
    divider: '#e0e0e0',
  },
  typography: {
    fontFamily: '"SF Pro Display", "Helvetica Neue", Arial, sans-serif',
    h4: {
      fontWeight: 300,
      letterSpacing: '-0.02em',
    },
    h6: {
      fontWeight: 500,
      letterSpacing: '-0.01em',
    },
    body1: {
      lineHeight: 1.6,
    },
  },
  components: {
    MuiPaper: {
      styleOverrides: {
        root: {
          backgroundColor: '#ffffff',
          border: '1px solid #e0e0e0',
          boxShadow: '0 2px 8px rgba(0,0,0,0.06)',
          borderRadius: 8,
        },
      },
    },
    MuiButton: {
      styleOverrides: {
        root: {
          textTransform: 'none',
          fontWeight: 500,
          borderRadius: 6,
          transition: 'all 0.2s ease-in-out',
        },
        contained: {
          backgroundColor: '#000000',
          color: '#ffffff',
          boxShadow: '0 2px 8px rgba(0,0,0,0.15)',
          '&:hover': {
            backgroundColor: '#333333',
            boxShadow: '0 4px 12px rgba(0,0,0,0.25)',
            transform: 'translateY(-1px)',
          },
        },
        outlined: {
          borderColor: '#000000',
          color: '#000000',
          borderWidth: '1.5px',
          '&:hover': {
            borderColor: '#333333',
            backgroundColor: 'rgba(0,0,0,0.04)',
            borderWidth: '1.5px',
          },
        },
      },
    },
    MuiTextField: {
      styleOverrides: {
        root: {
          '& .MuiOutlinedInput-root': {
            borderRadius: 6,
            '& fieldset': {
              borderColor: '#e0e0e0',
            },
            '&:hover fieldset': {
              borderColor: '#000000',
            },
            '&.Mui-focused fieldset': {
              borderColor: '#000000',
              borderWidth: '2px',
            },
          },
        },
      },
    },
    MuiToggleButton: {
      styleOverrides: {
        root: {
          borderRadius: 6,
          textTransform: 'none',
          fontWeight: 500,
          '&.Mui-selected': {
            backgroundColor: '#000000',
            color: '#ffffff',
            '&:hover': {
              backgroundColor: '#333333',
            },
          },
        },
      },
    },
    MuiAlert: {
      styleOverrides: {
        root: {
          borderRadius: 6,
        },
      },
    },
  },
});

function App() {
  const [uploadedFile, setUploadedFile] = useState(null);
  const [compressionResults, setCompressionResults] = useState(null);

  const handleFileUpload = (file) => {
    setUploadedFile(file);
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
        backgroundColor: '#ffffff',
        py: 4,
      }}>
        <Container maxWidth="lg">
          <Box sx={{ mb: 6, textAlign: 'center' }}>
            <Typography 
              variant="h4" 
              component="h1" 
              sx={{ 
                mb: 2,
                color: '#000000',
              }}
            >
              File Compression System
            </Typography>
            <Typography 
              variant="body1" 
              color="text.secondary"
              sx={{ maxWidth: 600, mx: 'auto' }}
            >
              High-performance compression algorithms: LZ77, Huffman, and RLE
            </Typography>
          </Box>

          <Grid container spacing={4}>
            <Grid item xs={12} md={6}>
              <Paper sx={{ p: 3, height: 'fit-content' }}>
                <FileUpload onFileSelect={handleFileUpload} />
              </Paper>
            </Grid>
            <Grid item xs={12} md={6}>
              <CompressionDashboard 
                file={uploadedFile}
                onCompressionComplete={handleCompressionComplete}
                results={compressionResults}
              />
            </Grid>
          </Grid>
        </Container>
      </Box>
    </ThemeProvider>
  );
}

export default App;
