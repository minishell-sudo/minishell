/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   export.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mohamibr <mohamibr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/15 15:13:03 by mohamibr          #+#    #+#             */
/*   Updated: 2024/12/26 11:11:39 by mohamibr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/minishell.h"

void	print_export(t_env_cpy *env_cpy)
{
	ft_printf("declare -x %s", env_cpy->type);
	if (env_cpy->equal == true)
		ft_printf("=\"%s\"\n", env_cpy->env);
	else
		ft_printf("\n");
	env_cpy = env_cpy->next;
}

t_env_cpy	*fill_token(t_env_cpy *env_cpy, char *str)
{
	char	*equal_sign_pos;
	char	*type;
	char	*env;
	bool	equal;

	env = NULL;
	equal = false;
	if (!str || !*str || str[0] == '{' || str[0] == '}')
		return (env_cpy);
	equal_sign_pos = ft_strchr(str, '=');
	if (equal_sign_pos)
	{
		type = ft_substr(str, 0, equal_sign_pos - str);
		env = equal_sign_pos + 1;
		equal = true;
	}
	else
		type = ft_strdup(str);
	if (!type)
		return (env_cpy);
	env_cpy = a_env(&env_cpy, type, env, equal);
	if (type)
		free(type);
	return (env_cpy);
}

int	check_ex(char *str)
{
	int	i;

	if (!str || str[0] == '\0')
		return (0);
	if (!((str[0] >= 'A' && str[0] <= 'Z') || (str[0] >= 'a' && str[0] <= 'z')
			|| str[0] == '_'))
		return (0);
	i = 1;
	while (str[i] && str[i] != '=')
	{
		if (!((str[i] >= 'A' && str[i] <= 'Z') || (str[i] >= 'a'
					&& str[i] <= 'z') || (str[i] >= '0' && str[i] <= '9')
				|| str[i] == '_'))
			return (0);
		i++;
	}
	return (1);
}

void	ft_export_loop(t_token *token, t_env_cpy *env_cpy, int *flag)
{
	if (token->tokens[0] != '{' && token->tokens[0] != '}')
	{
		if (check_ex(token->tokens))
		{
			env_cpy = fill_token(env_cpy, token->tokens);
			if (!env_cpy)
				*flag = 1;
		}
		else
		{
			*flag = 1;
			write_error("bash: export: ");
			write_error(token->tokens);
			write_error(": not a valid identifier\n");
			env_cpy->last_exit_status = 1;
		}
	}
}

void	ft_export(t_token *token, t_env_cpy *env_cpy)
{
	int	flag;

	flag = 0;
	if (!token->next)
	{
		env_cpy->last_exit_status = 0;
		print_sorted(env_cpy);
	}
	else
	{
		token = token->next;
		while (token)
		{
			ft_export_loop(token, env_cpy, &flag);
			token = token->next;
		}
	}
	if (flag == 0)
		env_cpy->last_exit_status = 0;
}
