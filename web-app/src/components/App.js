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
      main: '#1976d2',
    },
    secondary: {
      main: '#dc004e',
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
      <Container maxWidth="lg" sx={{ py: 4 }}>
        <Paper elevation={2} sx={{ p: 4, mb: 4 }}>
          <Typography variant="h4" component="h1" gutterBottom align="center">
            File Compression System
          </Typography>
          <Typography variant="body1" color="text.secondary" align="center">
            Upload any file and compress it using different algorithms
          </Typography>
        </Paper>

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

        {compressionResults && (
          <Box sx={{ mt: 3 }}>
            <Paper sx={{ p: 3 }}>
              <Typography variant="h6" gutterBottom>
                Compression Summary
              </Typography>
              <Typography variant="body2">
                Algorithm: <strong>{compressionResults.algorithm}</strong><br />
                Original Size: <strong>{(compressionResults.originalSize / 1024).toFixed(2)} KB</strong><br />
                Compressed Size: <strong>{(compressionResults.compressedSize / 1024).toFixed(2)} KB</strong><br />
                Compression Ratio: <strong>{compressionResults.ratio}%</strong><br />
                Time Taken: <strong>{compressionResults.time} ms</strong><br />
                Verified: <strong>{compressionResults.verified ? 'Yes' : 'No'}</strong>
              </Typography>
            </Paper>
          </Box>
        )}
      </Container>
    </ThemeProvider>
  );
}

export default App;
