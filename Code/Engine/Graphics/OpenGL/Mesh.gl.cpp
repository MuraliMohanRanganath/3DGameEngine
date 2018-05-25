#include "../Mesh.h"
#include "../../Windows/OpenGl.h"
#include "../../Asserts/Asserts.h"
#include "../../Logging/Logging.h"
#include "../../../External/OpenGlExtensions/OpenGlExtensions.h"
#include "../VertexData.h"

namespace {
	// The vertex buffer holds the data for each vertex
	GLuint s_vertexArrayId = 0;
#ifdef EAE6320_GRAPHICS_ISDEVICEDEBUGINFOENABLED
	// OpenGL debuggers don't seem to support freeing the vertex buffer
	// and letting the vertex array object hold a reference to it,
	// and so if debug info is enabled an explicit reference is held
	GLuint s_vertexBufferId = 0;
	GLuint s_indexBufferId = 0;
#endif
}

bool eae6320::Graphics::Mesh::Initialize() {
	bool wereThereErrors = false;
	GLuint vertexBufferId = 0;
	GLuint indexBufferID = 0;
	// Create a vertex array object and make it active
	{
		const GLsizei arrayCount = 1;
		glGenVertexArrays(arrayCount, &s_vertexArrayId);
		const GLenum errorCode = glGetError();
		if (errorCode == GL_NO_ERROR)
		{
			glBindVertexArray(s_vertexArrayId);
			const GLenum errorCode = glGetError();
			if (errorCode != GL_NO_ERROR)
			{
				wereThereErrors = true;
				EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
				eae6320::Logging::OutputError("OpenGL failed to bind the vertex array: %s",
					reinterpret_cast<const char*>(gluErrorString(errorCode)));
				goto OnExit;
			}
		}
		else
		{
			wereThereErrors = true;
			EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
			eae6320::Logging::OutputError("OpenGL failed to get an unused vertex array ID: %s",
				reinterpret_cast<const char*>(gluErrorString(errorCode)));
			goto OnExit;
		}
	}

	// Create a vertex buffer object and make it active
	{
		const GLsizei bufferCount = 1;
		glGenBuffers(bufferCount, &vertexBufferId);
		const GLenum errorCode = glGetError();
		if (errorCode == GL_NO_ERROR)
		{
			glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
			const GLenum errorCode = glGetError();
			if (errorCode != GL_NO_ERROR)
			{
				wereThereErrors = true;
				EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
				eae6320::Logging::OutputError("OpenGL failed to bind the vertex buffer: %s",
					reinterpret_cast<const char*>(gluErrorString(errorCode)));
				goto OnExit;
			}
		}
		else
		{
			wereThereErrors = true;
			EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
			eae6320::Logging::OutputError("OpenGL failed to get an unused vertex buffer ID: %s",
				reinterpret_cast<const char*>(gluErrorString(errorCode)));
			goto OnExit;
		}
	}
	
	// Create a index buffer object and make it active
	{
		const GLsizei indexBufferCount = 1;
		glGenBuffers(indexBufferCount, &s_indexBufferId);
		const GLenum errorCode = glGetError();
		if (errorCode == GL_NO_ERROR)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_indexBufferId);
			const GLenum errorCode = glGetError();
			if (errorCode != GL_NO_ERROR)
			{
				wereThereErrors = true;
				EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
				eae6320::Logging::OutputError("OpenGL failed to bind the index buffer: %s",
					reinterpret_cast<const char*>(gluErrorString(errorCode)));
				goto OnExit;
			}
		}
		else
		{
			wereThereErrors = true;
			EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
			eae6320::Logging::OutputError("OpenGL failed to get an unused index buffer ID: %s",
				reinterpret_cast<const char*>(gluErrorString(errorCode)));
			goto OnExit;
		}
	}

	// Assign the data to the buffer
	{
		const unsigned int bufferSize = verticesCount * sizeof(sVertex);

		glBufferData(GL_ARRAY_BUFFER, bufferSize, reinterpret_cast<GLvoid*>(vertexData),
			// In our class we won't ever read from the buffer
			GL_STATIC_DRAW);
		const GLenum errorCode = glGetError();
		if (errorCode != GL_NO_ERROR)
		{
			wereThereErrors = true;
			EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
			eae6320::Logging::OutputError("OpenGL failed to allocate the vertex buffer: %s",
				reinterpret_cast<const char*>(gluErrorString(errorCode)));
			goto OnExit;
		}
	}
	{
		const unsigned int indexBufferSize = indicesCount*sizeof(uint32_t);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, reinterpret_cast<GLvoid*>(indices),
			// In our class we won't ever read from the buffer
			GL_STATIC_DRAW);
		const GLenum indexBufferAllocateErrorCode = glGetError();
		if (indexBufferAllocateErrorCode != GL_NO_ERROR)
		{
			wereThereErrors = true;
			EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(indexBufferAllocateErrorCode)));
			eae6320::Logging::OutputError("OpenGL failed to allocate the index buffer: %s", reinterpret_cast<const char*>(gluErrorString(indexBufferAllocateErrorCode)));
			goto OnExit;
		}
	}

	// Initialize the vertex format
	{
		// The "stride" defines how large a single vertex is in the stream of data
		// (or, said another way, how far apart each position element is)
		const GLsizei stride = sizeof(sVertex);

		// Position (0)
		// 2 floats == 8 bytes
		// Offset = 0
		{
			const GLuint vertexElementLocation = 0;
			const GLint elementCount = 3;
			const GLboolean notNormalized = GL_FALSE;	// The given floats should be used as-is
			glVertexAttribPointer(vertexElementLocation, elementCount, GL_FLOAT, notNormalized, stride,
				reinterpret_cast<GLvoid*>(offsetof(sVertex, x)));
			const GLenum errorCode = glGetError();
			if (errorCode == GL_NO_ERROR)
			{
				glEnableVertexAttribArray(vertexElementLocation);
				const GLenum errorCode = glGetError();
				if (errorCode != GL_NO_ERROR)
				{
					wereThereErrors = true;
					EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
					eae6320::Logging::OutputError("OpenGL failed to enable the POSITION vertex attribute at location %u: %s",
						vertexElementLocation, reinterpret_cast<const char*>(gluErrorString(errorCode)));
					goto OnExit;
				}
			}
			else
			{
				wereThereErrors = true;
				EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
				eae6320::Logging::OutputError("OpenGL failed to set the POSITION vertex attribute at location %u: %s",
					vertexElementLocation, reinterpret_cast<const char*>(gluErrorString(errorCode)));
				goto OnExit;
			}
		}

		//color
		{
			const GLuint vertexElementLocation = 1;
			const GLint elementCount = 4;
			const GLboolean isNormalized = GL_TRUE;	
			glVertexAttribPointer(vertexElementLocation, elementCount, GL_UNSIGNED_BYTE, isNormalized, stride,
				reinterpret_cast<GLvoid*>(offsetof(sVertex, red)));
			const GLenum errorCode = glGetError();
			if (errorCode == GL_NO_ERROR)
			{
				glEnableVertexAttribArray(vertexElementLocation);
				const GLenum errorCode = glGetError();
				if (errorCode != GL_NO_ERROR)
				{
					wereThereErrors = true;
					EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
					eae6320::Logging::OutputError("OpenGL failed to enable the COLOR vertex attribute at location %u: %s",
						vertexElementLocation, reinterpret_cast<const char*>(gluErrorString(errorCode)));
					goto OnExit;
				}
			}
			else
			{
				wereThereErrors = true;
				EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
				eae6320::Logging::OutputError("OpenGL failed to set the COLOR vertex attribute at location %u: %s",
					vertexElementLocation, reinterpret_cast<const char*>(gluErrorString(errorCode)));
				goto OnExit;
			}
		}

		// Texture Coordinates
		{
			const GLuint vertexElementLocation = 2;
			const GLint elementCount = 2;
			const GLboolean notNormalized = GL_FALSE;	// The given floats should be used as-is
			glVertexAttribPointer(vertexElementLocation, elementCount, GL_FLOAT, notNormalized, stride,
				reinterpret_cast<GLvoid*>(offsetof(sVertex, u)));
			const GLenum errorCode = glGetError();
			if (errorCode == GL_NO_ERROR)
			{
				glEnableVertexAttribArray(vertexElementLocation);
				const GLenum errorCode = glGetError();
				if (errorCode != GL_NO_ERROR)
				{
					wereThereErrors = true;
					EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
					eae6320::Logging::OutputError("OpenGL failed to enable the Texture coordinate attribute at location %u: %s",
						vertexElementLocation, reinterpret_cast<const char*>(gluErrorString(errorCode)));
					goto OnExit;
				}
			}
			else
			{
				wereThereErrors = true;
				EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
				eae6320::Logging::OutputError("OpenGL failed to set the Texture coordinate attribute at location %u: %s",
					vertexElementLocation, reinterpret_cast<const char*>(gluErrorString(errorCode)));
				goto OnExit;
			}
		}


	}

OnExit:

	if (s_vertexArrayId != 0)
	{
		// Unbind the vertex array
		// (this must be done before deleting the vertex buffer)
		glBindVertexArray(0);
		const GLenum errorCode = glGetError();
		if (errorCode == GL_NO_ERROR)
		{
			// The vertex and index buffer objects can be freed
			// (the vertex array object will still hold references to them,
			// and so they won't actually goes away until it gets freed).
			// Unfortunately debuggers don't work well when these are freed
			// (gDEBugger just doesn't show anything and RenderDoc crashes),
			// and so don't free them if debug info is enabled.
			if (vertexBufferId != 0)
			{
#ifndef EAE6320_GRAPHICS_ISDEVICEDEBUGINFOENABLED
				const GLsizei bufferCount = 1;
				glDeleteBuffers(bufferCount, &vertexBufferId);
				const GLenum errorCode = glGetError();
				if (errorCode != GL_NO_ERROR)
				{
					wereThereErrors = true;
					EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
					eae6320::Logging::OutputError("OpenGL failed to delete vertex buffer: %s",
						reinterpret_cast<const char*>(gluErrorString(errorCode)));
					goto OnExit;
				}
				vertexBufferId = 0;
#else
				s_vertexBufferId = vertexBufferId;
#endif
			}
			if (s_indexBufferId != 0)
			{
#ifndef EAE6320_GRAPHICS_ISDEVICEDEBUGINFOENABLED
				const GLsizei bufferCount = 1;
				glDeleteBuffers(bufferCount, &indexBufferID);
				const GLenum errorCode = glGetError();
				if (errorCode != GL_NO_ERROR)
				{
					wereThereErrors = true;
					EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
					eae6320::Logging::OutputError("OpenGL failed to delete index buffer: %s",
						reinterpret_cast<const char*>(gluErrorString(errorCode)));
					goto OnExit;
				}
				indexBufferID = 0;
#else
				s_indexBufferId = indexBufferID;
#endif // !EAE6320_GRAPHICS_ISDEVICEDEBUGINFOENABLED
			}
		}
		else
		{
			wereThereErrors = true;
			EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
			eae6320::Logging::OutputError("OpenGL failed to unbind the vertex array: %s",
				reinterpret_cast<const char*>(gluErrorString(errorCode)));
			goto OnExit;
		}
	}

	return !wereThereErrors;
}

void eae6320::Graphics::Mesh::DrawFrame() {
	if (indicesCount == 0) {
		return;
	}
	// Bind a specific vertex buffer to the device as a data source
		glBindVertexArray(s_vertexArrayId);
		EAE6320_ASSERT(glGetError() == GL_NO_ERROR);
	
		// Render triangles from the currently-bound vertex buffer
		{
			// The mode defines how to interpret multiple vertices as a single "primitive";
			// we define a triangle list
			// (meaning that every primitive is a triangle and will be defined by three vertices)
			const GLenum mode = GL_TRIANGLES;
			
			// Every index is a 16 bit unsigned integer
			const GLenum indexType = GL_UNSIGNED_INT;
			// It's possible to start rendering primitives in the middle of the stream
			const GLvoid* const offset = 0;
			glDrawElements(mode, indicesCount, indexType, offset);

			EAE6320_ASSERT(glGetError() == GL_NO_ERROR);
		}
}

bool eae6320::Graphics::Mesh::CleanUp() {
	bool wereThereErrors = false;
#ifdef EAE6320_GRAPHICS_ISDEVICEDEBUGINFOENABLED
	if (s_vertexBufferId != 0)
	{
		const GLsizei bufferCount = 1;
		glDeleteBuffers(bufferCount, &s_vertexBufferId);
		const GLenum errorCode = glGetError();
		if (errorCode != GL_NO_ERROR)
		{
			wereThereErrors = true;
			EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
			Logging::OutputError("OpenGL failed to delete the vertex buffer: %s",
				reinterpret_cast<const char*>(gluErrorString(errorCode)));
		}
		s_vertexBufferId = 0;
	}
	if (s_indexBufferId != 0) {
		const GLsizei bufferCount = 1;
		glDeleteBuffers(bufferCount, &s_indexBufferId);
		const GLenum errorCode = glGetError();
		if (errorCode != GL_NO_ERROR)
		{
			wereThereErrors = true;
			EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
			Logging::OutputError("OpenGL failed to delete the index buffer: %s",
				reinterpret_cast<const char*>(gluErrorString(errorCode)));
		}
		s_indexBufferId = 0;
	}
#endif
	if (s_vertexArrayId != 0)
	{
		const GLsizei arrayCount = 1;
		glDeleteVertexArrays(arrayCount, &s_vertexArrayId);
		const GLenum errorCode = glGetError();
		if (errorCode != GL_NO_ERROR)
		{
			wereThereErrors = true;
			EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
			Logging::OutputError("OpenGL failed to delete the vertex array: %s",
				reinterpret_cast<const char*>(gluErrorString(errorCode)));
		}
		s_vertexArrayId = 0;
	}
	return !wereThereErrors;
}