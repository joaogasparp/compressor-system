import React, { useState } from 'react';
import { 
  ThemeProvider, 
  createTheme, 
  CssBaseline, 
  Container, 
  Typography, 
  Box,
  Grid
} from '@mui/material';
import FileUpload from './FileUpload';
import CompressionDashboard from './CompressionDashboard';
import '../override.css';

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
  shape: {
    borderRadius: 8,
  },
  shadows: [
    'none',
    'none',
    'none',
    'none',
    'none',
    'none',
    'none',
    'none',
    'none',
    'none',
    'none',
    'none',
    'none',
    'none',
    'none',
    'none',
    'none',
    'none',
    'none',
    'none',
    'none',
    'none',
    'none',
    'none',
    'none',
  ],
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
    MuiCssBaseline: {
      styleOverrides: {
        body: {
          '& .MuiPaper-root, & .MuiPaper-elevation, & .MuiPaper-rounded, & .MuiPaper-elevation1': {
            boxShadow: 'none !important',
            backgroundColor: '#ffffff !important',
            border: '1px solid #e0e0e0 !important',
            borderRadius: '8px !important',
          },
          '& [class*="MuiPaper"]': {
            boxShadow: 'none !important',
            backgroundColor: '#ffffff !important',
            border: '1px solid #e0e0e0 !important',
            borderRadius: '8px !important',
          },
          '& [class*="css-"]': {
            '&[class*="MuiPaper"]': {
              boxShadow: 'none !important',
              backgroundColor: '#ffffff !important',
              border: '1px solid #e0e0e0 !important',
              borderRadius: '8px !important',
            },
          },
        },
      },
    },
    MuiPaper: {
      styleOverrides: {
        root: {
          backgroundColor: '#ffffff !important',
          border: '1px solid #e0e0e0 !important',
          boxShadow: 'none !important',
          borderRadius: '8px !important',
          '&.MuiPaper-elevation': {
            boxShadow: 'none !important',
          },
          '&.MuiPaper-elevation1': {
            boxShadow: 'none !important',
          },
          '&.MuiPaper-rounded': {
            borderRadius: '8px !important',
          },
          // Remove all auto-generated CSS classes
          '&[class*="css-"]': {
            boxShadow: 'none !important',
            backgroundColor: '#ffffff !important',
            border: '1px solid #e0e0e0 !important',
            borderRadius: '8px !important',
          },
        },
        elevation0: {
          boxShadow: 'none !important',
        },
        elevation1: {
          boxShadow: 'none !important',
        },
        elevation2: {
          boxShadow: 'none !important',
        },
        elevation3: {
          boxShadow: 'none !important',
        },
        rounded: {
          borderRadius: '8px !important',
        },
      },
    },
    MuiButton: {
      styleOverrides: {
        root: {
          textTransform: 'none',
          fontWeight: 500,
          borderRadius: 6,
          transition: 'all 0.2s ease',
        },
        contained: {
          backgroundColor: '#000000',
          color: '#ffffff',
          boxShadow: '0 2px 4px rgba(0,0,0,0.2)',
          '&:hover': {
            backgroundColor: '#333333',
            boxShadow: '0 4px 8px rgba(0,0,0,0.3)',
          },
        },
        outlined: {
          borderColor: '#333333',
          color: '#333333',
          backgroundColor: '#ffffff',
          '&:hover': {
            borderColor: '#000000',
            backgroundColor: '#f5f5f5',
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
          backgroundColor: '#ffffff',
          border: '1px solid #d0d0d0',
          color: '#333333',
          '&:hover': {
            backgroundColor: '#f5f5f5',
            borderColor: '#333333',
          },
          '&.Mui-selected': {
            backgroundColor: '#000000',
            color: '#ffffff',
            borderColor: '#000000',
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
              <Box sx={{ 
                p: 3, 
                height: 'fit-content',
                backgroundColor: '#ffffff',
                border: '1px solid #e0e0e0',
                borderRadius: 8,
                boxShadow: 'none !important',
              }}>
                <FileUpload onFileSelect={handleFileUpload} />
              </Box>
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
