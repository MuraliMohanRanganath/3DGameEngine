#include "../ConstantBufferDataManager.h"
#include "../ConstantBufferData.h"
#include "../../Asserts/Asserts.h"
#include "../../Logging/Logging.h"
#include "../../Time/Time.h"

bool eae6320::Graphics::ConstantBufferDataManager::Initialize(ConstantBufferType bufferType, size_t  bufferSize, void * bufferData) {
	s_bufferType = bufferType;
	s_bufferSize = bufferSize;


	bool wereThereErrors = false;

	// Create a uniform buffer object and make it active
	{
		const GLsizei bufferCount = 1;
		glGenBuffers(bufferCount, &s_constantBufferId);
		const GLenum errorCode = glGetError();
		if (errorCode == GL_NO_ERROR)
		{
			glBindBuffer(GL_UNIFORM_BUFFER, s_constantBufferId);
			const GLenum errorCode = glGetError();
			if (errorCode != GL_NO_ERROR)
			{
				wereThereErrors = true;
				EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
				eae6320::Logging::OutputError("OpenGL failed to bind the new uniform buffer %u: %s",
					s_constantBufferId, reinterpret_cast<const char*>(gluErrorString(errorCode)));
				goto OnExit;
			}
		}
		else
		{
			wereThereErrors = true;
			EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
			eae6320::Logging::OutputError("OpenGL failed to get an unused uniform buffer ID: %s",
				reinterpret_cast<const char*>(gluErrorString(errorCode)));
			goto OnExit;
		}
	}
	// Fill in the constant buffer
//	s_constantBufferData.g_elapsedSecondCount_total = eae6320::Time::GetElapsedSecondCount_total();
	// Allocate space and copy the constant data into the uniform buffer
	{
		const GLenum usage = GL_DYNAMIC_DRAW;	// The buffer will be modified frequently and used to draw
		glBufferData(GL_UNIFORM_BUFFER, s_bufferSize, reinterpret_cast<const GLvoid*>(bufferData), usage);
		const GLenum errorCode = glGetError();
		if (errorCode != GL_NO_ERROR)
		{
			wereThereErrors = true;
			EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
			eae6320::Logging::OutputError("OpenGL failed to allocate the new uniform buffer %u: %s",
				s_constantBufferId, reinterpret_cast<const char*>(gluErrorString(errorCode)));
			goto OnExit;
		}
	}

OnExit:
	return !wereThereErrors;
}

bool eae6320::Graphics::ConstantBufferDataManager::Bind() {
	// Bind the constant buffer to the shader
	{
		const GLuint bindingPointAssignedInShader = s_bufferType;
		glBindBufferBase(GL_UNIFORM_BUFFER, bindingPointAssignedInShader, s_constantBufferId);
		EAE6320_ASSERT(glGetError() == GL_NO_ERROR);
	}
	return true;
}

bool eae6320::Graphics::ConstantBufferDataManager::CleanUp() {
	bool wereThereErrors = false;
	if (s_constantBufferId != 0)
	{
		const GLsizei bufferCount = 1;
		glDeleteBuffers(bufferCount, &s_constantBufferId);
		const GLenum errorCode = glGetError();
		if (errorCode != GL_NO_ERROR)
		{
			wereThereErrors = true;
			EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
			Logging::OutputError("OpenGL failed to delete the constant buffer: %s",
				reinterpret_cast<const char*>(gluErrorString(errorCode)));
		}
		s_constantBufferId = 0;
	}
	return !wereThereErrors;
}

bool eae6320::Graphics::ConstantBufferDataManager::Update(void* bufferData) {
	// Update the struct (i.e. the memory that we own)
	//s_constantBufferId.g_elapsedSecondCount_total = Time::GetElapsedSecondCount_total();
	
	// Make the uniform buffer active
	{
		glBindBuffer(GL_UNIFORM_BUFFER, s_constantBufferId);
		EAE6320_ASSERT(glGetError() == GL_NO_ERROR);
	}
	// Copy the updated memory to the GPU
	{
		GLintptr updateAtTheBeginning = 0;
		GLsizeiptr updateTheEntireBuffer = static_cast<GLsizeiptr>(s_bufferSize);
		glBufferSubData(GL_UNIFORM_BUFFER, updateAtTheBeginning, updateTheEntireBuffer, bufferData);
		EAE6320_ASSERT(glGetError() == GL_NO_ERROR);
	}
	return true;
}