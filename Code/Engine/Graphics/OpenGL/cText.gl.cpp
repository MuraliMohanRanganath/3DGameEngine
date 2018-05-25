#include "../cText.h"
#include "../VertexData.h"
#include "../../Asserts/Asserts.h"
#include "../../Logging/Logging.h"
#include "../../UserSettings/UserSettings.h"

GLuint eae6320::Graphics::cText::ms_vertexArrayId = 0;
GLuint eae6320::Graphics::cText::ms_vertexBufferId = 0;
GLuint eae6320::Graphics::cText::ms_indexBufferId = 0;

void eae6320::Graphics::cText::Draw() const
{
	const size_t length = strlen(m_text);
	const unsigned int vertexCount = 4 * length;
	const unsigned int indexCount = 6 * length;

	const float widthMultiplier = 2.0f / UserSettings::GetResolutionWidth();
	const float heightMultiplier = 2.0f / UserSettings::GetResolutionHeight();

	sScreenPosition* screenPosForEachQuad = reinterpret_cast<sScreenPosition*>(malloc(length * sizeof(sScreenPosition)));
	for (size_t i = 0; i < length; i++)
	{
		screenPosForEachQuad[i].top = (float)m_y*heightMultiplier;
		screenPosForEachQuad[i].bottom = (float)(m_y - 32)*heightMultiplier;
		if (i > 0)
		{
			screenPosForEachQuad[i].left = screenPosForEachQuad[i - 1].right + 5.0f * widthMultiplier;
		}
		else
		{
			screenPosForEachQuad[i].left = (float)m_x*widthMultiplier;
		}
		screenPosForEachQuad[i].right = screenPosForEachQuad[i].left + ((static_cast<int32_t>(m_Font[m_text[i] - 32].size) + 10)*widthMultiplier);
	}
	
	sVertex* vertexData = reinterpret_cast<sVertex*>(malloc(vertexCount * sizeof(sVertex)));
	{
		for (size_t i = 0; i < length; i++)
		{
			vertexData[i * 4 + 0].x = screenPosForEachQuad[i].left; 
			vertexData[i * 4 + 0].y = screenPosForEachQuad[i].bottom;
			vertexData[i * 4 + 0].z = 0;
			vertexData[i * 4 + 0].u = m_Font[m_text[i] - 32].left;
			vertexData[i * 4 + 0].v = 0.02f;
			vertexData[i * 4 + 0].red = vertexData[i * 4 + 0].green = vertexData[i * 4 + 0].blue = vertexData[i * 4 + 0].alpha = 255;

			vertexData[i * 4 + 1].x = screenPosForEachQuad[i].right;
			vertexData[i * 4 + 1].y = screenPosForEachQuad[i].bottom;
			vertexData[i * 4 + 1].z = 0;
			vertexData[i * 4 + 1].u = m_Font[m_text[i] - 32].right;
			vertexData[i * 4 + 1].v = 0.02f;
			vertexData[i * 4 + 1].red = vertexData[i * 4 + 1].green = vertexData[i * 4 + 1].blue = vertexData[i * 4 + 1].alpha = 255;

			vertexData[i * 4 + 2].x = screenPosForEachQuad[i].left;
			vertexData[i * 4 + 2].y = screenPosForEachQuad[i].top;
			vertexData[i * 4 + 2].z = 0;
			vertexData[i * 4 + 2].u = m_Font[m_text[i] - 32].left;
			vertexData[i * 4 + 2].v = 1;
			vertexData[i * 4 + 2].red = vertexData[i * 4 + 2].green = vertexData[i * 4 + 2].blue = vertexData[i * 4 + 2].alpha = 255;

			vertexData[i * 4 + 3].x = screenPosForEachQuad[i].right;
			vertexData[i * 4 + 3].y = screenPosForEachQuad[i].top;
			vertexData[i * 4 + 3].z = 0;
			vertexData[i * 4 + 3].u = m_Font[m_text[i] - 32].right;
			vertexData[i * 4 + 3].v = 1;
			vertexData[i * 4 + 3].red = vertexData[i * 4 + 3].green = vertexData[i * 4 + 3].blue = vertexData[i * 4 + 3].alpha = 255;
		}

		uint32_t* indexData = reinterpret_cast<uint32_t*>(malloc(indexCount * sizeof(uint32_t)));
		for (size_t i = 0; i < length; i++)
		{
			indexData[i * 6 + 0] = static_cast<uint32_t>(i * 4 + 0);
			indexData[i * 6 + 1] = static_cast<uint32_t>(i * 4 + 1);
			indexData[i * 6 + 2] = static_cast<uint32_t>(i * 4 + 2);
			indexData[i * 6 + 3] = static_cast<uint32_t>(i * 4 + 3);
			indexData[i * 6 + 4] = static_cast<uint32_t>(i * 4 + 2);
			indexData[i * 6 + 5] = static_cast<uint32_t>(i * 4 + 1);
		}

		// Make the vertex buffer active
		{
			glBindBuffer(GL_ARRAY_BUFFER, ms_vertexBufferId);
			EAE6320_ASSERT(glGetError() == GL_NO_ERROR);
		}
		// Invalidate the old data
		{
			// This is to tell OpenGL that synchronization isn't necessary
			// (it can finish drawing with the contents of the previous buffer,
			// but there's no need to wait for the next update before drawing anything else)
			glInvalidateBufferData(ms_vertexBufferId);
			EAE6320_ASSERT(glGetError() == GL_NO_ERROR);
		}
		// Re-allocate and copy the new data to the GPU
		{
			const unsigned int vertexBufferSize = vertexCount * sizeof(sVertex);
			glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, reinterpret_cast<GLvoid*>(vertexData), GL_STREAM_DRAW);
			EAE6320_ASSERT(glGetError() == GL_NO_ERROR);
		}

		// Make the index buffer active
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ms_indexBufferId);
			EAE6320_ASSERT(glGetError() == GL_NO_ERROR);
		}
		// Invalidate the old data
		{
			// This is to tell OpenGL that synchronization isn't necessary
			// (it can finish drawing with the contents of the previous buffer,
			// but there's no need to wait for the next update before drawing anything else)
			glInvalidateBufferData(ms_indexBufferId);
			EAE6320_ASSERT(glGetError() == GL_NO_ERROR);
		}
		// Re-allocate and copy the new data to the GPU
		{
			const unsigned int indexBufferSize = indexCount * sizeof(uint32_t);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, reinterpret_cast<GLvoid*>(indexData), GL_STREAM_DRAW);
			EAE6320_ASSERT(glGetError() == GL_NO_ERROR);
		}

		// Bind the vertex buffer to the device as a data source
		{
			glBindVertexArray(ms_vertexArrayId);
			EAE6320_ASSERT(glGetError() == GL_NO_ERROR);
		}
		// Render triangles from the currently-bound vertex buffer
		{
			// The mode defines how to interpret multiple vertices as a single "primitive";
			// we have defined the vertex buffer as a quad, which means that a triangle "strip" will work
			// (meaning that the first primitive will be a triangle defined by three vertices
			// and the second primitive will be a triangle defined by the two most recent vertices and one new vertex)
			const GLenum mode = GL_TRIANGLES;//GL_TRIANGLE_STRIP;
			GLenum indexType = indexType = GL_UNSIGNED_INT;
			// It's possible to start rendering primitives in the middle of the stream

			const GLvoid* const offset = 0;
			glDrawElements(mode, indexCount, indexType, offset);
			/*const GLint indexOfFirstVertexToRender = 0;
			glDrawArrays(mode, indexOfFirstVertexToRender, static_cast<GLsizei>(vertexCount));*/
			EAE6320_ASSERT(glGetError() == GL_NO_ERROR);
		}
	}
}

bool eae6320::Graphics::cText::Initialize()
{
	bool wereThereErrors = false;

	// Store the vertex data in a vertex buffer
	{
		// Create a vertex array object and make it active
		{
			const GLsizei arrayCount = 1;
			glGenVertexArrays(arrayCount, &ms_vertexArrayId);
			const GLenum errorCode = glGetError();
			if (errorCode == GL_NO_ERROR)
			{
				glBindVertexArray(ms_vertexArrayId);
				const GLenum errorCode = glGetError();
				if (errorCode != GL_NO_ERROR)
				{
					wereThereErrors = true;
					EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
					eae6320::Logging::OutputError("OpenGL failed to bind the sprites' new vertex array: %s",
						reinterpret_cast<const char*>(gluErrorString(errorCode)));
					goto OnExit;
				}
			}
			else
			{
				wereThereErrors = true;
				EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
				eae6320::Logging::OutputError("OpenGL failed to get an unused vertex array ID for sprites: %s",
					reinterpret_cast<const char*>(gluErrorString(errorCode)));
				goto OnExit;
			}
		}

		// Create a vertex buffer object and make it active
		{
			const GLsizei bufferCount = 1;
			glGenBuffers(bufferCount, &ms_vertexBufferId);
			const GLenum errorCode = glGetError();
			if (errorCode == GL_NO_ERROR)
			{
				glBindBuffer(GL_ARRAY_BUFFER, ms_vertexBufferId);
				const GLenum errorCode = glGetError();
				if (errorCode != GL_NO_ERROR)
				{
					wereThereErrors = true;
					EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
					eae6320::Logging::OutputError("OpenGL failed to bind the sprites' new vertex buffer: %s",
						reinterpret_cast<const char*>(gluErrorString(errorCode)));
					goto OnExit;
				}
			}
			else
			{
				wereThereErrors = true;
				EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
				eae6320::Logging::OutputError("OpenGL failed to get an unused vertex buffer ID for the sprites: %s",
					reinterpret_cast<const char*>(gluErrorString(errorCode)));
				goto OnExit;
			}
		}
		// Assign initial data to the buffer
		{
			sVertex vertexData[4];
			{
				// Lower Left
				{
					sVertex& vertex = vertexData[0];
					vertex.x = -1.0f;
					vertex.y = -1.0f;
					vertex.z = -1.0f;
					vertex.u = 0.0f;
					vertex.v = 0.0f;
					vertex.red = vertex.green = vertex.blue = vertex.alpha = 255;
				}
				// Lower Right
				{
					sVertex& vertex = vertexData[1];
					vertex.x = 1.0f;
					vertex.y = -1.0f;
					vertex.z = -1.0f;
					vertex.u = 1.0f;
					vertex.v = 0.0f;
					vertex.red = vertex.green = vertex.blue = vertex.alpha = 255;
				}
				// Upper Left
				{
					sVertex& vertex = vertexData[2];
					vertex.x = -1.0f;
					vertex.y = 1.0f;
					vertex.z = -1.0f;
					vertex.u = 0.0f;
					vertex.v = 1.0f;
					vertex.red = vertex.green = vertex.blue = vertex.alpha = 255;
				}
				// Upper Right
				{
					sVertex& vertex = vertexData[3];
					vertex.x = 1.0f;
					vertex.y = 1.0f;
					vertex.z = -1.0f;
					vertex.u = 1.0f;
					vertex.v = 1.0f;
					vertex.red = vertex.green = vertex.blue = vertex.alpha = 255;
				}
			}
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), reinterpret_cast<const GLvoid*>(vertexData),
				// The buffer will change frequently, and each update will only be used once for a draw call
				GL_STREAM_DRAW);
			const GLenum errorCode = glGetError();
			if (errorCode != GL_NO_ERROR)
			{
				wereThereErrors = true;
				EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
				eae6320::Logging::OutputError("OpenGL failed to allocate the sprites' vertex buffer: %s",
					reinterpret_cast<const char*>(gluErrorString(errorCode)));
				goto OnExit;
			}
		}
		// Initialize the vertex format
		{
			// This code is the same as in the mesh representation :(
			// If I didn't have to try and make it easy for you all to update
			// I would move this to a shared function
			const GLsizei stride = sizeof(sVertex);
			// Position (0)
			// 3 floats == 12 bytes
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
			// Color (1)
			// 4 uint8_ts == 4 bytes
			// Offset = 12
			{
				const GLuint vertexElementLocation = 1;
				const GLint elementCount = 4;
				// Each element will be sent to the GPU as an unsigned byte in the range [0,255]
				// but these values should be understood as representing [0,1] values
				// and that is what the shader code will interpret them as
				// (in other words, we could change the values provided here in C code
				// to be floats and sent GL_FALSE instead and the shader code wouldn't need to change)
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
						eae6320::Logging::OutputError("OpenGL failed to enable the COLOR0 vertex attribute at location %u: %s",
							vertexElementLocation, reinterpret_cast<const char*>(gluErrorString(errorCode)));
						goto OnExit;
					}
				}
				else
				{
					wereThereErrors = true;
					EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
					eae6320::Logging::OutputError("OpenGL failed to set the COLOR0 vertex attribute at location %u: %s",
						vertexElementLocation, reinterpret_cast<const char*>(gluErrorString(errorCode)));
					goto OnExit;
				}
			}
			// Texcoord (0)
			// 2 floats == 8 bytes
			// Offset = 16
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
						eae6320::Logging::OutputError("OpenGL failed to enable the TEXCOORD0 vertex attribute at location %u: %s",
							vertexElementLocation, reinterpret_cast<const char*>(gluErrorString(errorCode)));
						goto OnExit;
					}
				}
				else
				{
					wereThereErrors = true;
					EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
					eae6320::Logging::OutputError("OpenGL failed to set the TEXCOORD0 vertex attribute at location %u: %s",
						vertexElementLocation, reinterpret_cast<const char*>(gluErrorString(errorCode)));
					goto OnExit;
				}
			}
		}

		// Create a index buffer object and make it active
		{
			const GLsizei bufferCount = 1;
			glGenBuffers(bufferCount, &ms_indexBufferId);
			const GLenum errorCode = glGetError();
			if (errorCode == GL_NO_ERROR)
			{
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ms_indexBufferId);
				const GLenum errorCode = glGetError();
				if (errorCode != GL_NO_ERROR)
				{
					wereThereErrors = true;
					EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
					eae6320::Logging::OutputError("OpenGL failed to bind fonts' index buffer: %s",
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

		{
			uint32_t indexData[6] = { 0,1,2,3,2,1 };
			//Index Buffer init
			const unsigned int indexBufferSize = 6 * sizeof(uint32_t);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, reinterpret_cast<GLvoid*>(indexData), GL_STREAM_DRAW);
			const GLenum errorCode = glGetError();
			if (errorCode != GL_NO_ERROR)
			{
				wereThereErrors = true;
				EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
				eae6320::Logging::OutputError("OpenGL failed to allocate fonts' index buffer: %s",
					reinterpret_cast<const char*>(gluErrorString(errorCode)));
				goto OnExit;
			}
		}
	}

OnExit:

	return !wereThereErrors;
}

bool eae6320::Graphics::cText::CleanUp()
{
	bool wereThereErrors = false;

	// Make sure that the vertex array isn't bound
	{
		// Unbind the vertex array
		glBindVertexArray(0);
		const GLenum errorCode = glGetError();
		if (errorCode != GL_NO_ERROR)
		{
			wereThereErrors = true;
			EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
			Logging::OutputError("OpenGL failed to unbind all vertex arrays before deleting cleaning up the sprites: %s",
				reinterpret_cast<const char*>(gluErrorString(errorCode)));
		}
	}
	if (ms_vertexBufferId != 0)
	{
		const GLsizei bufferCount = 1;
		glDeleteBuffers(bufferCount, &ms_vertexBufferId);
		const GLenum errorCode = glGetError();
		if (errorCode != GL_NO_ERROR)
		{
			wereThereErrors = true;
			EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
			Logging::OutputError("OpenGL failed to delete the sprites' vertex buffer: %s",
				reinterpret_cast<const char*>(gluErrorString(errorCode)));
		}
		ms_vertexBufferId = 0;
	}
	if (ms_vertexArrayId != 0)
	{
		const GLsizei arrayCount = 1;
		glDeleteVertexArrays(arrayCount, &ms_vertexArrayId);
		const GLenum errorCode = glGetError();
		if (errorCode != GL_NO_ERROR)
		{
			EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
			Logging::OutputError("OpenGL failed to delete the sprites' vertex array: %s",
				reinterpret_cast<const char*>(gluErrorString(errorCode)));
		}
		ms_vertexArrayId = 0;
	}

	if (ms_indexBufferId != 0)
	{
		const GLsizei arrayCount = 1;
		glDeleteVertexArrays(arrayCount, &ms_indexBufferId);
		const GLenum errorCode = glGetError();
		if (errorCode != GL_NO_ERROR)
		{
			EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
			Logging::OutputError("OpenGL failed to delete the sprites' index array: %s",
				reinterpret_cast<const char*>(gluErrorString(errorCode)));
		}
		ms_indexBufferId = 0;
	}
	return !wereThereErrors;
}