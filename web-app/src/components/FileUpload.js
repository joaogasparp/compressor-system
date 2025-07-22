import React, { useState, useCallback } from 'react';
import {
  Box,
  Typography,
  Button,
  Paper,
  Alert,
  Chip,
} from '@mui/material';
import {
  CloudUpload as UploadIcon,
  InsertDriveFile as FileIcon,
  CheckCircle as CheckIcon,
} from '@mui/icons-material';

const FileUpload = ({ onFileSelect }) => {
  const [file, setFile] = useState(null);
  const [dragOver, setDragOver] = useState(false);
  const [error, setError] = useState('');

  const handleFileChange = useCallback((selectedFile) => {
    if (!selectedFile) return;

    // Basic validation - 20MB limit
    if (selectedFile.size > 20 * 1024 * 1024) {
      setError('File size must be less than 20MB');
      return;
    }

    setFile(selectedFile);
    setError('');
    onFileSelect(selectedFile);
  }, [onFileSelect]);

  const handleDrop = useCallback((e) => {
    e.preventDefault();
    setDragOver(false);
    
    const files = e.dataTransfer.files;
    if (files.length > 0) {
      handleFileChange(files[0]);
    }
  }, [handleFileChange]);

  const handleDragOver = useCallback((e) => {
    e.preventDefault();
    setDragOver(true);
  }, []);

  const handleDragLeave = useCallback((e) => {
    e.preventDefault();
    setDragOver(false);
  }, []);

  const formatFileSize = (bytes) => {
    if (bytes === 0) return '0 Bytes';
    const k = 1024;
    const sizes = ['Bytes', 'KB', 'MB', 'GB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
  };

  return (
    <Box>
      <Typography variant="h6" gutterBottom sx={{ fontWeight: 500, mb: 3 }}>
        Select File
      </Typography>

      {error && (
        <Alert severity="error" sx={{ mb: 2, borderRadius: 1 }}>
          {error}
        </Alert>
      )}

      <Paper
        onDrop={handleDrop}
        onDragOver={handleDragOver}
        onDragLeave={handleDragLeave}
        sx={{
          p: 4,
          textAlign: 'center',
          cursor: 'pointer',
          border: dragOver ? '2px dashed #000000' : '2px dashed #e0e0e0',
          backgroundColor: dragOver ? 'rgba(0,0,0,0.02)' : '#fafafa',
          borderRadius: 2,
          transition: 'all 0.3s ease',
          '&:hover': {
            borderColor: '#000000',
            backgroundColor: 'rgba(0,0,0,0.02)',
            transform: 'translateY(-2px)',
            boxShadow: '0 4px 12px rgba(0,0,0,0.1)',
          },
        }}
        component="label"
        htmlFor="file-upload"
      >
        <input
          id="file-upload"
          type="file"
          hidden
          onChange={(e) => handleFileChange(e.target.files[0])}
        />
        
        {!file ? (
          <>
            <UploadIcon sx={{ fontSize: 48, color: 'text.secondary', mb: 2 }} />
            <Typography variant="h6" gutterBottom sx={{ fontWeight: 400 }}>
              Drop file here or click to select
            </Typography>
            <Typography variant="body2" color="text.secondary">
              Maximum file size: 20MB
            </Typography>
          </>
        ) : (
          <Box>
            <CheckIcon sx={{ fontSize: 48, color: 'success.main', mb: 2 }} />
            <Typography variant="h6" gutterBottom sx={{ fontWeight: 500 }}>
              File Selected
            </Typography>
            
            <Box sx={{ 
              display: 'flex', 
              alignItems: 'center', 
              justifyContent: 'center',
              gap: 2,
              mt: 2,
              mb: 2
            }}>
              <FileIcon sx={{ color: 'text.secondary' }} />
              <Box sx={{ textAlign: 'left' }}>
                <Typography variant="body1" sx={{ fontWeight: 500 }}>
                  {file.name}
                </Typography>
                <Typography variant="body2" color="text.secondary">
                  {formatFileSize(file.size)}
                </Typography>
              </Box>
            </Box>

            <Chip
              label={file.type || 'Unknown type'}
              size="small"
              sx={{ mb: 2 }}
            />

            <Button
              variant="outlined"
              component="label"
              htmlFor="file-upload"
              sx={{ mt: 2 }}
            >
              Select Different File
            </Button>
          </Box>
        )}
      </Paper>
    </Box>
  );
};

export default FileUpload;
