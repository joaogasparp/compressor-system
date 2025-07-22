import React, { useState, useCallback } from 'react';
import {
  Box,
  Typography,
  Button,
  Alert,
  Chip,
} from '@mui/material';
import {
  CloudUpload as UploadIcon,
  InsertDriveFile as FileIcon,
  CheckCircle as CheckIcon,
  UploadFile as SimpleUploadIcon,
  Add as AddIcon,
  FileUpload as FileUploadIcon,
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

      <Box
        onDrop={handleDrop}
        onDragOver={handleDragOver}
        onDragLeave={handleDragLeave}
        sx={{
          p: 3,
          textAlign: 'center',
          cursor: 'pointer',
          borderRadius: 8,
          transition: 'all 0.2s ease',
          boxShadow: 'none !important',
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'center',
          minHeight: 220,
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
          <Box sx={{ 
            display: 'flex',
            flexDirection: 'column',
            alignItems: 'center',
            justifyContent: 'center',
            width: 525,
            height: 180,
            margin: '0 auto',
            borderRadius: '21px',
            border: '2px dashed #cccccc',
            backgroundColor: '#fafafa',
            transition: 'all 0.2s ease',
            ...(dragOver && {
              borderColor: '#333333',
              backgroundColor: '#f5f5f5',
            }),
          }}>
            <Box sx={{
              fontSize: '32px',
              mb: 1.5,
              lineHeight: 1,
            }}>
              📁
            </Box>
            
            <Typography variant="h6" sx={{ 
              fontWeight: 500, 
              mb: 1, 
              color: '#000000',
              fontSize: '1rem',
              textAlign: 'center',
              px: 2,
            }}>
              Choose a file to upload
            </Typography>
            <Typography variant="body2" sx={{ 
              color: '#666666',
              fontWeight: 400,
              fontSize: '0.8rem',
              textAlign: 'center',
              px: 2,
            }}>
              Drag and drop or click to browse
            </Typography>
            <Typography variant="caption" sx={{ 
              color: '#999999',
              fontSize: '0.7rem',
              mt: 0.5,
            }}>
              Max 20MB
            </Typography>
          </Box>
        ) : (
          <Box>
            <Box sx={{ 
              display: 'flex',
              justifyContent: 'center',
              alignItems: 'center',
              mb: 2,
            }}>
              <CheckIcon sx={{ 
                fontSize: 48, 
                color: '#2e7d32',
                display: 'block',
              }} />
            </Box>
            <Typography variant="h6" sx={{ fontWeight: 500, mb: 1.5, color: '#000000' }}>
              File Selected
            </Typography>
            
            <Box sx={{ 
              display: 'flex', 
              alignItems: 'center', 
              justifyContent: 'center',
              gap: 2,
              mb: 1.5,
              p: 2,
              backgroundColor: '#f5f5f5',
              borderRadius: 6,
              border: '1px solid #e0e0e0',
            }}>
              <FileIcon sx={{ 
                color: '#666666',
              }} />
              <Box sx={{ textAlign: 'left' }}>
                <Typography variant="body1" sx={{ 
                  fontWeight: 500,
                  color: '#000000',
                }}>
                  {file.name}
                </Typography>
                <Typography variant="body2" sx={{
                  color: '#666666',
                }}>
                  {formatFileSize(file.size)}
                </Typography>
              </Box>
            </Box>

            <Chip
              label={file.type || 'Unknown type'}
              size="small"
              sx={{ 
                mb: 1.5,
                backgroundColor: '#f0f0f0',
                border: '1px solid #d0d0d0',
                color: '#333333',
              }}
            />

            <Button
              variant="outlined"
              component="label"
              htmlFor="file-upload"
              sx={{ 
                borderColor: '#333333',
                color: '#333333',
                backgroundColor: '#ffffff',
                '&:hover': {
                  borderColor: '#000000',
                  backgroundColor: '#f5f5f5',
                },
              }}
            >
              Select Different File
            </Button>
          </Box>
        )}
      </Box>
    </Box>
  );
};

export default FileUpload;
