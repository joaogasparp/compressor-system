import React, { useState } from 'react';
import {
  Container,
  Paper,
  Typography,
  Box,
  AppBar,
  Toolbar,
  Grid,
} from '@mui/material';
import { Archive as ArchiveIcon } from '@mui/icons-material';
import FileUploader from './components/FileUploader';
import CompressionDashboard from './components/CompressionDashboard';

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
    <Box sx={{ flexGrow: 1 }}>
      <AppBar position="static" elevation={2}>
        <Toolbar>
          <ArchiveIcon sx={{ mr: 2 }} />
          <Typography variant="h6" component="div" sx={{ flexGrow: 1 }}>
            Sistema de Compressão de Arquivos
          </Typography>
        </Toolbar>
      </AppBar>

      <Container maxWidth="lg" sx={{ mt: 4, mb: 4 }}>
        <Grid container spacing={3}>
          {/* Header */}
          <Grid item xs={12}>
            <Paper sx={{ p: 3, textAlign: 'center' }}>
              <Typography variant="h4" gutterBottom>
                Compressor System Dashboard
              </Typography>
              <Typography variant="body1" color="text.secondary">
                Faça upload de arquivos e comprima usando diferentes algoritmos:
                RLE, Huffman, ou LZ77
              </Typography>
            </Paper>
          </Grid>

          {/* File Upload Section */}
          <Grid item xs={12} md={6}>
            <FileUploader 
              onFileUpload={handleFileUpload}
              uploadedFile={uploadedFile}
            />
          </Grid>

          {/* Compression Dashboard */}
          <Grid item xs={12} md={6}>
            <CompressionDashboard 
              file={uploadedFile}
              onCompressionComplete={handleCompressionComplete}
              results={compressionResults}
            />
          </Grid>

          {/* Results Section */}
          {compressionResults && (
            <Grid item xs={12}>
              <Paper sx={{ p: 3 }}>
                <Typography variant="h6" gutterBottom>
                  Resultados da Compressão
                </Typography>
                <Grid container spacing={2}>
                  <Grid item xs={12} sm={6} md={3}>
                    <Box sx={{ textAlign: 'center', p: 2, bgcolor: 'primary.light', borderRadius: 1 }}>
                      <Typography variant="h4" color="primary.contrastText">
                        {compressionResults.algorithm}
                      </Typography>
                      <Typography variant="body2" color="primary.contrastText">
                        Algoritmo
                      </Typography>
                    </Box>
                  </Grid>
                  <Grid item xs={12} sm={6} md={3}>
                    <Box sx={{ textAlign: 'center', p: 2, bgcolor: 'secondary.light', borderRadius: 1 }}>
                      <Typography variant="h4" color="secondary.contrastText">
                        {compressionResults.ratio}%
                      </Typography>
                      <Typography variant="body2" color="secondary.contrastText">
                        Taxa de Compressão
                      </Typography>
                    </Box>
                  </Grid>
                  <Grid item xs={12} sm={6} md={3}>
                    <Box sx={{ textAlign: 'center', p: 2, bgcolor: 'success.light', borderRadius: 1 }}>
                      <Typography variant="h4" color="success.contrastText">
                        {compressionResults.time}ms
                      </Typography>
                      <Typography variant="body2" color="success.contrastText">
                        Tempo
                      </Typography>
                    </Box>
                  </Grid>
                  <Grid item xs={12} sm={6} md={3}>
                    <Box sx={{ textAlign: 'center', p: 2, bgcolor: 'info.light', borderRadius: 1 }}>
                      <Typography variant="h4" color="info.contrastText">
                        {compressionResults.verified ? '✓' : '✗'}
                      </Typography>
                      <Typography variant="body2" color="info.contrastText">
                        Verificado
                      </Typography>
                    </Box>
                  </Grid>
                </Grid>
              </Paper>
            </Grid>
          )}
        </Grid>
      </Container>
    </Box>
  );
}

export default App;
