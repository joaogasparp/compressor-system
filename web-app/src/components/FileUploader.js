import React, { useCallback } from 'react';
import { useDropzone } from 'react-dropzone';
import {
  Paper,
  Typography,
  Box,
  Chip,
  List,
  ListItem,
  ListItemText,
  ListItemIcon,
  LinearProgress,
} from '@mui/material';
import {
  CloudUpload as CloudUploadIcon,
  InsertDriveFile as FileIcon,
  CheckCircle as CheckIcon,
} from '@mui/icons-material';

const FileUploader = ({ onFileUpload, uploadedFile }) => {
  const onDrop = useCallback((acceptedFiles) => {
    if (acceptedFiles.length > 0) {
      onFileUpload(acceptedFiles[0]);
    }
  }, [onFileUpload]);

  const { getRootProps, getInputProps, isDragActive } = useDropzone({
    onDrop,
    multiple: false,
  });

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
        Upload de Arquivo
      </Typography>

      <Box
        {...getRootProps()}
        sx={{
          border: '2px dashed',
          borderColor: isDragActive ? 'primary.main' : 'grey.300',
          borderRadius: 2,
          p: 3,
          textAlign: 'center',
          cursor: 'pointer',
          bgcolor: isDragActive ? 'action.hover' : 'background.paper',
          transition: 'all 0.3s ease',
          '&:hover': {
            borderColor: 'primary.main',
            bgcolor: 'action.hover',
          },
        }}
      >
        <input {...getInputProps()} />
        <CloudUploadIcon sx={{ fontSize: 48, color: 'grey.400', mb: 2 }} />
        <Typography variant="body1" gutterBottom>
          {isDragActive
            ? 'Solte o arquivo aqui...'
            : 'Clique ou arraste um arquivo para fazer upload'}
        </Typography>
        <Typography variant="body2" color="text.secondary">
          Suporta qualquer tipo de arquivo
        </Typography>
      </Box>

      {uploadedFile && (
        <Box sx={{ mt: 3 }}>
          <Typography variant="subtitle2" gutterBottom>
            Arquivo Carregado:
          </Typography>
          <List dense>
            <ListItem>
              <ListItemIcon>
                <FileIcon color="primary" />
              </ListItemIcon>
              <ListItemText
                primary={uploadedFile.name}
                secondary={formatFileSize(uploadedFile.size)}
              />
              <CheckIcon color="success" />
            </ListItem>
          </List>
          
          <Box sx={{ mt: 2, display: 'flex', gap: 1, flexWrap: 'wrap' }}>
            <Chip
              label={`Tamanho: ${formatFileSize(uploadedFile.size)}`}
              size="small"
              color="primary"
              variant="outlined"
            />
            <Chip
              label={`Tipo: ${uploadedFile.type || 'Desconhecido'}`}
              size="small"
              color="secondary"
              variant="outlined"
            />
          </Box>
        </Box>
      )}
    </Paper>
  );
};

export default FileUploader;
