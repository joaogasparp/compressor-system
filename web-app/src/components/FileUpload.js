import React, { useCallback } from 'react';
import {
  Paper,
  Typography,
  Box,
  Button,
  Alert,
  Chip,
} from '@mui/material';
import {
  CloudUpload as UploadIcon,
  InsertDriveFile as FileIcon,
} from '@mui/icons-material';

const FileUpload = ({ onFileSelect, selectedFile }) => {
  const handleFileChange = useCallback((event) => {
    const file = event.target.files[0];
    if (file) {
      onFileSelect(file);
    }
  }, [onFileSelect]);

  const handleDragOver = useCallback((event) => {
    event.preventDefault();
  }, []);

  const handleDrop = useCallback((event) => {
    event.preventDefault();
    const files = event.dataTransfer.files;
    if (files.length > 0) {
      onFileSelect(files[0]);
    }
  }, [onFileSelect]);

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
        File Upload
      </Typography>

      <Box
        onDragOver={handleDragOver}
        onDrop={handleDrop}
        sx={{
          border: '2px dashed',
          borderColor: selectedFile ? 'success.main' : 'grey.300',
          borderRadius: 2,
          p: 4,
          textAlign: 'center',
          backgroundColor: selectedFile ? 'success.50' : 'grey.50',
          cursor: 'pointer',
          transition: 'all 0.3s ease',
          '&:hover': {
            borderColor: 'primary.main',
            backgroundColor: 'primary.50',
          },
        }}
      >
        <input
          accept="*"
          style={{ display: 'none' }}
          id="file-upload"
          type="file"
          onChange={handleFileChange}
        />
        <label htmlFor="file-upload">
          <UploadIcon sx={{ fontSize: 48, color: 'text.secondary', mb: 2 }} />
          <Typography variant="h6" gutterBottom>
            {selectedFile ? 'File Selected' : 'Drop file here or click to select'}
          </Typography>
          <Typography variant="body2" color="text.secondary">
            Supports any file type
          </Typography>
        </label>
      </Box>

      {selectedFile && (
        <Box sx={{ mt: 2 }}>
          <Alert severity="success" sx={{ mb: 2 }}>
            File successfully selected
          </Alert>
          
          <Box sx={{ display: 'flex', alignItems: 'center', gap: 1, mb: 2 }}>
            <FileIcon color="primary" />
            <Typography variant="body1" sx={{ fontWeight: 'medium' }}>
              {selectedFile.name}
            </Typography>
          </Box>

          <Box sx={{ display: 'flex', gap: 1, flexWrap: 'wrap' }}>
            <Chip
              label={`Size: ${formatFileSize(selectedFile.size)}`}
              variant="outlined"
              color="primary"
            />
            <Chip
              label={`Type: ${selectedFile.type || 'Unknown'}`}
              variant="outlined"
              color="secondary"
            />
          </Box>

          <Button
            variant="outlined"
            fullWidth
            sx={{ mt: 2 }}
            component="label"
            htmlFor="file-upload"
          >
            Select Different File
          </Button>
        </Box>
      )}
    </Paper>
  );
};

export default FileUpload;
