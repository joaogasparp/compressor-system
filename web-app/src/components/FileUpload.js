import React, { useCallback, useState } from 'react';
import {
  Paper,
  Typography,
  Box,
  Button,
  Alert,
  Chip,
  LinearProgress,
  Fade,
} from '@mui/material';
import {
  CloudUpload as UploadIcon,
  InsertDriveFile as FileIcon,
  CheckCircle as CheckIcon,
} from '@mui/icons-material';

const FileUpload = ({ onFileSelect, selectedFile }) => {
  const [isDragging, setIsDragging] = useState(false);
  const [isUploading, setIsUploading] = useState(false);

  const handleFileChange = useCallback(async (event) => {
    const file = event.target.files[0];
    if (file) {
      setIsUploading(true);
      // Simulate upload delay for better UX
      await new Promise(resolve => setTimeout(resolve, 500));
      onFileSelect(file);
      setIsUploading(false);
    }
  }, [onFileSelect]);

  const handleDragOver = useCallback((event) => {
    event.preventDefault();
    setIsDragging(true);
  }, []);

  const handleDragLeave = useCallback((event) => {
    event.preventDefault();
    setIsDragging(false);
  }, []);

  const handleDrop = useCallback(async (event) => {
    event.preventDefault();
    setIsDragging(false);
    const files = event.dataTransfer.files;
    if (files.length > 0) {
      setIsUploading(true);
      await new Promise(resolve => setTimeout(resolve, 500));
      onFileSelect(files[0]);
      setIsUploading(false);
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
    <Paper 
      sx={{ 
        p: 3, 
        height: '100%',
        background: 'rgba(255, 255, 255, 0.95)',
        backdropFilter: 'blur(10px)',
        borderRadius: 3
      }}
    >
      <Typography variant="h6" gutterBottom sx={{ fontWeight: 600, mb: 3 }}>
        File Upload
      </Typography>

      <Box
        onDragOver={handleDragOver}
        onDragLeave={handleDragLeave}
        onDrop={handleDrop}
        sx={{
          border: '3px dashed',
          borderColor: isDragging ? 'primary.main' : (selectedFile ? 'success.main' : 'grey.300'),
          borderRadius: 3,
          p: 4,
          textAlign: 'center',
          backgroundColor: isDragging ? 'primary.50' : (selectedFile ? 'success.50' : 'grey.50'),
          cursor: 'pointer',
          transition: 'all 0.3s cubic-bezier(0.4, 0, 0.2, 1)',
          position: 'relative',
          overflow: 'hidden',
          '&:hover': {
            borderColor: 'primary.main',
            backgroundColor: 'primary.50',
            transform: 'scale(1.02)',
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
        <label htmlFor="file-upload" style={{ cursor: 'pointer', width: '100%', height: '100%' }}>
          {isUploading ? (
            <Box>
              <UploadIcon sx={{ fontSize: 48, color: 'primary.main', mb: 2 }} />
              <Typography variant="h6" gutterBottom color="primary">
                Processing...
              </Typography>
              <LinearProgress sx={{ mt: 2, borderRadius: 1 }} />
            </Box>
          ) : selectedFile ? (
            <Fade in={true}>
              <Box>
                <CheckIcon sx={{ fontSize: 48, color: 'success.main', mb: 2 }} />
                <Typography variant="h6" gutterBottom color="success.main">
                  File Ready
                </Typography>
                <Typography variant="body2" color="text.secondary">
                  Click to select a different file
                </Typography>
              </Box>
            </Fade>
          ) : (
            <Box>
              <UploadIcon sx={{ fontSize: 48, color: 'text.secondary', mb: 2 }} />
              <Typography variant="h6" gutterBottom>
                {isDragging ? 'Drop your file here' : 'Drop file here or click to select'}
              </Typography>
              <Typography variant="body2" color="text.secondary">
                Supports any file type • Max size: 100MB
              </Typography>
            </Box>
          )}
        </label>
      </Box>

      {selectedFile && (
        <Fade in={true}>
          <Box sx={{ mt: 3 }}>
            <Alert 
              severity="success" 
              sx={{ 
                mb: 2,
                borderRadius: 2,
                '& .MuiAlert-icon': {
                  fontSize: '1.2rem'
                }
              }}
            >
              File successfully selected and ready for compression
            </Alert>
            
            <Box sx={{ 
              display: 'flex', 
              alignItems: 'center', 
              gap: 1, 
              mb: 2,
              p: 2,
              borderRadius: 2,
              bgcolor: 'grey.100'
            }}>
              <FileIcon color="primary" />
              <Box sx={{ flex: 1 }}>
                <Typography variant="body1" sx={{ fontWeight: 600, fontSize: '0.9rem' }}>
                  {selectedFile.name}
                </Typography>
                <Typography variant="body2" color="text.secondary" sx={{ fontSize: '0.8rem' }}>
                  {selectedFile.type || 'Unknown type'}
                </Typography>
              </Box>
            </Box>

            <Box sx={{ display: 'flex', gap: 1, flexWrap: 'wrap', mb: 2 }}>
              <Chip
                label={`${formatFileSize(selectedFile.size)}`}
                variant="filled"
                color="primary"
                size="small"
              />
              <Chip
                label={`Modified: ${new Date(selectedFile.lastModified).toLocaleDateString()}`}
                variant="outlined"
                color="secondary"
                size="small"
              />
            </Box>

            <Button
              variant="outlined"
              fullWidth
              sx={{ 
                mt: 2,
                borderRadius: 2,
                py: 1.5
              }}
              component="label"
              htmlFor="file-upload"
            >
              Select Different File
            </Button>
          </Box>
        </Fade>
      )}
    </Paper>
  );
};

export default FileUpload;
